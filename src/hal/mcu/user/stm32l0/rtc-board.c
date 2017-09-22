/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include <math.h>
#include <time.h>
#include "board.h"
#include "rtc-board.h"

//#if 0
//// 32768Hz
//#define RTC_ALARM_TICK_DURATION                     (0.12207031)  // unit: ms, 1 tick every 122us
//#define RTC_ALARM_TICK_PER_MS                       (8.192) // 1/8.192 = tick duration in ms
//#define RTC_SUBSEC_MAX                              (8192u)
//#else
//// 32774Hz
//#define RTC_ALARM_TICK_DURATION                     (0.12204796) // unit: ms, 1 tick every 122us
//#define RTC_ALARM_TICK_PER_MS                       (8.1935)      // 1/8.192 = tick duration in ms
//#define RTC_SUBSEC_MAX                              (8194u)
//#endif

/*!
 * Maximum number of days that can be handled by the RTC alarm counter before overflow.
 */
#define RTC_ALARM_MAX_NUMBER_OF_DAYS                (28)
#define RTC_50MS_TICKS                              ((uint32_t) (50.0/RTC_ALARM_TICK_DURATION) )
#define RTC_400US_TICKS                             ((uint32_t) (0.4/RTC_ALARM_TICK_DURATION) )

uint32_t RtcOffset = 0;
uint32_t RTC_ALARM_MAX_MS       = 2418757016;

#if 1
double RTC_ALARM_TICK_DURATION  = 0.12204796;
double RTC_ALARM_TICK_PER_MS    = 8.1935;
#define RTC_SUBSEC_MAX_CONST      (8194)
#else
double RTC_ALARM_TICK_DURATION  = 0.12207031;
double RTC_ALARM_TICK_PER_MS    = 8.192;
#define RTC_SUBSEC_MAX_CONST      (8192)
#endif

uint8_t RTC_PREDIV_A            = 4;
uint32_t RTC_SUBSEC_MAX         = RTC_SUBSEC_MAX_CONST;
uint32_t SubsecsInSecond        = RTC_SUBSEC_MAX_CONST*1;
uint32_t SubsecsInMinute        = RTC_SUBSEC_MAX_CONST*60;
uint32_t SubsecsInHour          = RTC_SUBSEC_MAX_CONST*60*60;
uint32_t SubsecsInDay           = RTC_SUBSEC_MAX_CONST*60*60*24;

/*!
 * Number of seconds in a minute
 */
static const uint8_t SecondsInMinute = 60;

/*!
 * Number of seconds in an hour
 */
static const uint16_t SecondsInHour = 3600;

/*!
 * Number of seconds in a day
 */
static const uint32_t SecondsInDay = 86400;

/*!
 * Number of hours in a day
 */
static const uint8_t HoursInDay = 24;

/*!
 * Number of seconds in a leap year
 */
static const uint32_t SecondsInLeapYear = 31622400;

/*!
 * Number of seconds in a year
 */
static const uint32_t SecondsInYear = 31536000;

/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Holds the current century for real time computation
 */
//static uint16_t Century = 0;

/*!
 * Flag used to indicates a Calendar Roll Over is about to happen
 */
//static bool CallendarRollOverReady = false;

/*!
 * Flag used to indicates a the MCU has waken-up from an external IRQ
 */
volatile bool NonScheduledWakeUp = false;

/*!
 * Current RTC timer context
 */
RtcCalendar_t RtcCalendarContext;

/*!
 * \brief Flag to indicate if the timestamps until the next event is long enough
 * to set the MCU into low power mode
 */
static bool RtcTimerEventAllowsLowPower = false;

/*!
 * \brief Flag to disable the LowPower Mode even if the timestamps until the
 * next event is long enough to allow Low Power mode
 */
static bool LowPowerDisableDuringTask = false;

/*!
 * \brief RTC Handler
 */
//RTC_HandleTypeDef RtcHandle = { 0 };

/*!
 * \brief Indicates if the RTC is already Initialized or not
 */
static bool RtcInitalized = false;

/*!
 * \brief Indicates if the RTC Wake Up Time is calibrated or not
 */
static bool WakeUpTimeInitialized = false;

/*!
 * \brief Hold the Wake-up time duration in ticks
 */
volatile uint32_t McuWakeUpTime = 0;

/*!
 * \brief Hold the cumulated error in micro-second to compensate the timing errors
 */
static int32_t TimeoutValueError = 0;

/*!
 * \brief RTC wakeup time computation
 */
static void RtcComputeWakeUpTime( void );

/*!
 * \brief Start the RTC Alarm (timeoutValue is in ms)
 */
static void RtcStartWakeUpAlarm( uint32_t timeoutValue );

/*!
 * \brief Converts a TimerTime_t value into RtcCalendar_t value
 *
 * \param[IN] timeCounter Value to convert to RTC calendar
 * \retval rtcCalendar New RTC calendar value
 */
//
// REMARK: Removed function static attribute in order to suppress
//         "#177-D function was declared but never referenced" warning.
// static RtcCalendar_t RtcConvertTimerTimeToCalendarTick( TimerTime_t timeCounter )
//
void RtcConvertTimerTimeToCalendarTick( TimerTime_t timeCounter, RtcCalendar_t *calendar );

/*!
 * \brief Converts a RtcCalendar_t value into TimerTime_t value
 *
 * \param[IN/OUT] calendar Calendar value to be converted
 *                         [NULL: compute from "now",
 *                          Others: compute from given calendar value]
 * \retval timerTime New TimerTime_t value
 */
//static TimerTime_t RtcConvertCalendarTickToTimerTime( RtcCalendar_t *calendar );

/*!
 * \brief Converts a TimerTime_t value into a value for the RTC Alarm
 *
 * \param[IN] timeCounter Value in ms to convert into a calendar alarm date
 * \param[IN] now Current RTC calendar context
 * \retval rtcCalendar Value for the RTC Alarm
 */
static void RtcComputeTimerTimeToAlarmTick( TimerTime_t timeCounter, RtcCalendar_t *now, RtcCalendar_t *alarmTimer );

/*!
 * \brief Returns the internal RTC Calendar and check for RTC overflow
 *
 * \retval calendar RTC calendar
 */
static void RtcGetCalendar( RtcCalendar_t *cal );

/*!
 * \brief Check the status for the calendar year to increase the value of Century at the overflow of the RTC
 *
 * \param[IN] year Calendar current year
 */
//static void RtcCheckCalendarRollOver( uint8_t year );

static void RTC_DisalbeAlarmA( void )
{
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->CR &= ~(RTC_CR_ALRAIE | RTC_CR_ALRAE);
    while((RTC->ISR & RTC_ISR_ALRAWF) != RTC_ISR_ALRAWF);
    RTC->WPR = 0xFE;
    RTC->WPR = 0x64;
}

#define RTC_TR_RESERVED_MASK    ((uint32_t)0x007F7F7F)
#define RTC_DR_RESERVED_MASK    ((uint32_t)0x00FFFF3F)
#define RTC_RSF_MASK            ((uint32_t)0xFFFFFF5F)
#define SYNCHRO_TIMEOUT          ((uint32_t) 0x00008000)
void RTC_WaitForSynchro(void)
{
    __IO uint32_t synchrocounter = 0;
    uint32_t synchrostatus = 0x00;

    /* Disable the write protection for RTC registers */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    /* Clear RSF flag */
    RTC->ISR &= (uint32_t)RTC_RSF_MASK;

    /* Wait the registers to be synchronised */
    do
    {
        synchrostatus = RTC->ISR & RTC_ISR_RSF;
        synchrocounter++;
    } while((synchrocounter != SYNCHRO_TIMEOUT) && (synchrostatus == 0x00));

    /* Enable the write protection for RTC registers */
    RTC->WPR = 0xFF;
}

static uint8_t Bcd2ToByte(uint8_t Value)
{
    uint8_t tmp = 0;
    tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
    return (tmp + (Value & (uint8_t)0x0F));
}

static uint8_t ByteToBcd2(uint8_t Value)
{
    uint8_t bcdhigh = 0;

    while (Value >= 10)
    {
        bcdhigh++;
        Value -= 10;
    }

    return  ((uint8_t)(bcdhigh << 4) | Value);
}

void RtcInit( void )
{
    if( RtcInitalized == false ){
        /** DBP enable */
        RCC->APB1ENR |= RCC_APB1ENR_PWREN;
        PWR->CR |= PWR_CR_DBP;

        /** reset RTC */
        RCC->CSR |= RCC_CSR_RTCRST;
        RCC->CSR &= ~RCC_CSR_RTCRST;

        /** enable LSE */
        RCC->CSR |= RCC_CSR_LSEON;
        while((RCC->CSR & RCC_CSR_LSERDY)!=RCC_CSR_LSERDY){
            /* add time out here for a robust application */
        }

        /** Enable RTC, use LSE as clock */
        RCC->CSR = (RCC->CSR & ~RCC_CSR_RTCSEL) | RCC_CSR_RTCEN | RCC_CSR_RTCSEL_0;

        //RCC->APB1ENR &=~ RCC_APB1ENR_PWREN;

        RTC_DisalbeAlarmA();

        /* Configure exti and nvic for RTC IT */
        /* (13) unmask line 17 */
        /* (14) Rising edge for line 17 */
        /* (15) Set priority */
        /* (16) Enable RTC_IRQn */
        EXTI->IMR |= EXTI_IMR_IM17; /* (13) */
        EXTI->RTSR |= EXTI_RTSR_TR17; /* (14) */
        NVIC_SetPriority(RTC_IRQn, 0); /* (15) */
        NVIC_EnableIRQ(RTC_IRQn); /* (16) */

        /* RTC init mode */
        /* Configure RTC */
        /* (1) Write access for RTC registers */
        /* (2) Enable init phase */
        /* (3) Wait until it is allow to modify RTC register values */
        /* (4) set prescaler, 40kHz/64 => 625 Hz, 625Hz/625 => 1Hz */
        /* (5) Set time to 0 */
        /* (6) Disable init phase */
        /* (7) Disable write access for RTC registers */
        RTC->WPR = 0xCA; /* (1) */
        RTC->WPR = 0x53; /* (1) */
        RTC->ISR |= RTC_ISR_INIT; /* (2) */
        while((RTC->ISR & RTC_ISR_INITF)!=RTC_ISR_INITF) /* (3) */
        {
        /* add time out here for a robust application */
        }
        //RTC->PRER = 0x00010001; /* (4) */
        RTC->PRER = ((RTC_PREDIV_A - 1)<<16)|(RTC_SUBSEC_MAX-1); /* (4) */
        RTC->TR = 0; /* (5) */
        // 2000-01-01 Friday
        RTC->DR = 0x00002101;
        RTC->CR = (RTC->CR & ~RTC_CR_FMT);
        RTC->CR |= RTC_CR_BYPSHAD;
        //RTC->CR &= ~RTC_CR_BYPSHAD;
        RTC->ISR &=~ RTC_ISR_INIT; /* (6) */
        RTC->WPR = 0xFE; /* (7) */
        RTC->WPR = 0x64; /* (7) */

        RtcInitalized = true;
    }
}

void RtcSetTimeout( uint32_t timeout )
{
    RtcStartWakeUpAlarm( timeout );
}

TimerTime_t RtcGetAdjustedTimeoutValue( uint32_t timeout )
{
////    if( timeout > McuWakeUpTime )
////    {   // we have waken up from a GPIO and we have lost "McuWakeUpTime" that we need to compensate on next event
////        if( NonScheduledWakeUp == true )
////        {
////            NonScheduledWakeUp = false;
////            timeout -= McuWakeUpTime;
////        }
////    }
//
//    if( timeout > McuWakeUpTime )
//    {   // we don't go in Low Power mode for delay below 50ms (needed for LEDs)
//        if( timeout < 50 ) // 50 ms
//        {
//            RtcTimerEventAllowsLowPower = false;
//        }
//        else
//        {
//            RtcTimerEventAllowsLowPower = true;
////            timeout -= McuWakeUpTime;
//            if( NonScheduledWakeUp == true )
//            {
//                NonScheduledWakeUp = false;
//                timeout -= McuWakeUpTime;
//            }
//        }
//    }
    return  timeout;
}

TimerTime_t RtcGetTimerValue( void )
{
    return RtcGetTick(MS, NULL);
}

TimerTime_t RtcGetElapsedAlarmTime( void )
{
    TimerTime_t currentTime = 0;
    TimerTime_t contextTime = 0;
    TimerTime_t diff;

    currentTime = RtcGetTick( MS, NULL );
    contextTime = RtcGetTick( MS, &RtcCalendarContext );

    diff =  currentTime - contextTime;

    return diff;
}

TimerTime_t RtcComputeFutureEventTime( TimerTime_t futureEventInTime )
{
    return( RtcGetTimerValue( ) + futureEventInTime );
}

TimerTime_t RtcComputeElapsedTime( TimerTime_t eventInTime )
{
    TimerTime_t elapsedTime = 0;

    // Needed at boot, cannot compute with 0 or elapsed time will be equal to current time
    if( eventInTime == 0 )
    {
        return 0;
    }

    elapsedTime = RtcGetTick(MS, NULL);

    return ( elapsedTime - eventInTime );
}

void BlockLowPowerDuringTask ( bool status )
{
    if( status == true )
    {
        RtcRecoverMcuStatus( );
    }
    LowPowerDisableDuringTask = status;
}

void RtcEnterLowPowerStopMode( void )
{
    if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
    {
        /** disable irq */
        __disable_irq( );

        //rtc_lowpower_flag = true;

        NonScheduledWakeUp = false;

        /** Deinit board */
        BoardDeInitMcu( );

        /* Enable PWR clock */
        RCC->APB1ENR |= (uint32_t)(RCC_APB1ENR_PWREN);

        /* Set SLEEPDEEP bit of Cortex System Control Register */
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

        /* Clear WakeUp flag before enter sleep mode */
        PWR->CR |= (uint32_t)(PWR_CR_CWUF);

        /* Disable the Power Voltage Detector */
        PWR->CR &= (uint32_t)(~PWR_CR_PVDE);

        /* Set MCU in ULP (Ultra Low Power) */
        PWR->CR |= PWR_CR_ULP;

        /*Enable fast wakeUp*/
        PWR->CR |= (uint32_t)(PWR_CR_FWU);

        /* Enter Stop mode(not standby mode) when mcu enters deepsleep */
        PWR->CR &= (uint32_t)(~PWR_CR_PDDS);

        /* Regulator is in low power mode */
        PWR->CR |= PWR_CR_LPSDSR;

        /* Request Wait For Interrupt */
        __WFI();

        /** enable irq */
        __enable_irq( );

        /* Reset SLEEPDEEP bit of Cortex System Control Register */
        SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP_Msk);
    }
}

void RtcRecoverMcuStatus( void )
{
    if( PWR->CR & PWR_CR_ULP ){
        // PWR_CSR_WUF indicates the Alarm has waken-up the MCU
        // TODO: This code is doubted no use
        if( (PWR->CSR & PWR_CSR_WUF) == PWR_CSR_WUF){
            PWR->CR |= PWR_CR_CWUF;
        }else{

        }

        if( ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_MSI) ||
        ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_HSI) ){
            NonScheduledWakeUp = true;
        }

        /** disable irq */
        __disable_irq( );

        /** Reinitial clock */
        //clock_init(0);

        /* Enable PWR Clock */
        RCC->APB1ENR |= (RCC_APB1ENR_PWREN);

        /* WeakUp flag must be cleared before enter sleep again */
        PWR->CR |= (uint32_t)(PWR_CR_CWUF);
        while( (PWR->CSR & PWR_CSR_WUF) == PWR_CSR_WUF );

        /* Enable the Power Voltage Detector */
        PWR->CR |= (uint32_t)(PWR_CR_PVDE);

        /* Exit MCU in ULP (Ultra Low Power) */
        PWR->CR &= (uint32_t)~PWR_CR_ULP;

        BoardInitMcu();

        /** enable irq */
        __enable_irq( );
    }
}

static void RtcComputeWakeUpTime( void )
{
    uint32_t tmpreg = 0;
    uint16_t rtcAlarmSubsecs = 0;
    uint8_t rtcAlarmSeconds = 0;
    uint8_t rtcAlarmMinutes = 0;
    uint8_t rtcAlarmHours = 0;
    //uint16_t rtcAlarmDays = 0;

    uint32_t start = 0;
    uint32_t stop = 0;

    RtcCalendar_t now;

    if( WakeUpTimeInitialized == false )
    {
        RtcGetCalendar( &now );

        tmpreg = RTC->ALRMAR;
        //rtcAlarmDays = Bcd2ToByte((uint32_t)((tmpreg & (RTC_ALRMAR_DT | RTC_ALRMAR_DU)) >> 24));
        rtcAlarmHours = Bcd2ToByte((uint32_t)((tmpreg & (RTC_ALRMAR_HT | RTC_ALRMAR_HU)) >> 16));
        rtcAlarmMinutes = Bcd2ToByte((uint32_t)((tmpreg & (RTC_ALRMAR_MNT | RTC_ALRMAR_MNU)) >> 8));
        rtcAlarmSeconds = Bcd2ToByte((uint32_t)(tmpreg & (RTC_ALRMAR_ST | RTC_ALRMAR_SU)));
        rtcAlarmSubsecs = (RTC_SUBSEC_MAX - 1) - (uint16_t)(RTC->ALRMASSR&0xFFFF);

        start = rtcAlarmSubsecs + SubsecsInSecond * rtcAlarmSeconds + SubsecsInMinute * rtcAlarmMinutes + SubsecsInHour * rtcAlarmHours ;

        stop =  now.CalendarTime.Subseconds + SubsecsInSecond * now.CalendarTime.Seconds + \
                SubsecsInMinute * now.CalendarTime.Minutes + SubsecsInHour * now.CalendarTime.Hours;

        //McuWakeUpTime = ceil ( ( stop - start ) * (RTC_ALARM_TICK_DURATION ) );
        McuWakeUpTime = stop - start;

        WakeUpTimeInitialized = true;
    }
}

static void RtcStartWakeUpAlarm( uint32_t timeoutValue )
{
    uint32_t tmpreg = 0;
    RtcCalendar_t now;
    RtcCalendar_t alarmTimer;

    uint16_t rtcAlarmSubsecs = 0;
    uint8_t rtcAlarmSeconds = 0;
    uint8_t rtcAlarmMinutes = 0;
    uint8_t rtcAlarmHours = 0;
    uint16_t rtcAlarmDays = 0;

    RTC_DisalbeAlarmA();

    RtcGetCalendar( &RtcCalendarContext );
    now = RtcCalendarContext;

    RtcComputeTimerTimeToAlarmTick( timeoutValue, &now, &alarmTimer );

    rtcAlarmSubsecs = alarmTimer.CalendarTime.Subseconds;
    rtcAlarmSeconds = alarmTimer.CalendarTime.Seconds;
    rtcAlarmMinutes = alarmTimer.CalendarTime.Minutes;
    rtcAlarmHours = alarmTimer.CalendarTime.Hours;
    rtcAlarmDays = alarmTimer.CalendarDate.Date;
    tmpreg = (((uint32_t)ByteToBcd2(rtcAlarmHours) << 16) | \
              ((uint32_t)ByteToBcd2(rtcAlarmMinutes) << 8) | \
              ((uint32_t)ByteToBcd2(rtcAlarmSeconds)) | \
              ((uint32_t)ByteToBcd2(rtcAlarmDays) << 24));

    /* Disable the write protection for RTC registers */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    /* Configure the Alarm register */
    RTC->ALRMAR = (uint32_t)tmpreg;
    RTC->ALRMASSR =  0x0F000000 | ( (RTC_SUBSEC_MAX-1) - rtcAlarmSubsecs );  // Compare all 15bits
    /* Enable the write protection for RTC registers */
    RTC->WPR = 0xFF;

    /* Wait for RTC APB registers synchronisation */
    //RTC_WaitForSynchro( );

    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->CR |= (RTC_CR_ALRAIE | RTC_CR_ALRAE);
    RTC->WPR = 0xFE;
    RTC->WPR = 0x64;
}

static void RtcComputeTimerTimeToAlarmTick( TimerTime_t timeCounter, RtcCalendar_t *now, RtcCalendar_t *alarmTimer )
{
    //RtcCalendar_t calendar = now;

    uint16_t subsecs = now->CalendarTime.Subseconds;
    uint16_t seconds = now->CalendarTime.Seconds;
    uint16_t minutes = now->CalendarTime.Minutes;
    uint16_t hours = now->CalendarTime.Hours;
    uint16_t days = now->CalendarDate.Date;
    double timeoutValueTemp = 0.0;
    double timeoutValue = 0.0;
    double error = 0.0;

    //timeCounter = MIN( timeCounter, ( TimerTime_t )( RTC_ALARM_MAX_NUMBER_OF_DAYS * SubsecsInDay * RTC_ALARM_TICK_DURATION ) );
    timeCounter = MIN( timeCounter, RTC_ALARM_MAX_MS );

    if( timeCounter < 1 )
    {
        timeCounter = 1;
    }

    // timeoutValue is used for complete computation, timeoutvalue is in ticks now.
    timeoutValue = round( timeCounter * RTC_ALARM_TICK_PER_MS );
    if( timeoutValue > McuWakeUpTime )
    {   // we don't go in Low Power mode for delay below 50ms (needed for LEDs)
        if( timeoutValue < RTC_50MS_TICKS ) // 50 ms
        {
            RtcTimerEventAllowsLowPower = false;
        }
        else
        {
            RtcTimerEventAllowsLowPower = true;
            if( NonScheduledWakeUp == true )
            {
                NonScheduledWakeUp = false;
                timeoutValue -= McuWakeUpTime;
            }
            else
            {
                timeoutValue -= RTC_400US_TICKS;
            }
        }
    }

    // timeoutValueTemp is used to compensate the cumulating errors in timing far in the future
    timeoutValueTemp =  ( double )timeCounter * RTC_ALARM_TICK_PER_MS;

    // Compute timeoutValue error
    error = timeoutValue - timeoutValueTemp;

    // Add new error value to the cumulated value in uS
    TimeoutValueError += ( error  * 1000 );

    // Correct cumulated error if greater than ( RTC_ALARM_TICK_DURATION * 1000 )
    if( TimeoutValueError >= ( int32_t )( RTC_ALARM_TICK_DURATION * 1000 ) )
    {
        TimeoutValueError = TimeoutValueError - ( uint32_t )( RTC_ALARM_TICK_DURATION * 1000 );
        timeoutValue = timeoutValue + 1;
    }

    // Convert ticks to RTC format and add to now
    while( timeoutValue >= SubsecsInDay )
    {
        timeoutValue -= SubsecsInDay;
        days++;
    }

    // Calculate hours
    while( timeoutValue >= SubsecsInHour )
    {
        timeoutValue -= SubsecsInHour;
        hours++;
    }

    // Calculate minutes
    while( timeoutValue >= SubsecsInMinute )
    {
        timeoutValue -= SubsecsInMinute;
        minutes++;
    }

    // Convert milliseconds to RTC format and add to now
    while( timeoutValue >= SubsecsInSecond )
    {
        timeoutValue -= SubsecsInSecond;
        seconds++;
    }

    // Calculate seconds
    subsecs = (uint16_t)(subsecs + timeoutValue);

    while(subsecs >= RTC_SUBSEC_MAX){
        subsecs -= RTC_SUBSEC_MAX;
        seconds++;
    }

    // Correct for modulo
    while( seconds >= 60 )
    {
        seconds -= 60;
        minutes++;
    }

    while( minutes >= 60 )
    {
        minutes -= 60;
        hours++;
    }

    while( hours >= HoursInDay )
    {
        hours -= HoursInDay;
        days++;
    }

    if( ( now->CalendarDate.Year == 0 ) || ( now->CalendarDate.Year % 4 ) == 0 )
    {
        if( days > DaysInMonthLeapYear[now->CalendarDate.Month - 1] )
        {
            days = days % DaysInMonthLeapYear[now->CalendarDate.Month - 1];
        }
    }
    else
    {
        if( days > DaysInMonth[now->CalendarDate.Month - 1] )
        {
            days = days % DaysInMonth[now->CalendarDate.Month - 1];
        }
    }

    alarmTimer->CalendarTime.Subseconds = subsecs;
    alarmTimer->CalendarTime.Seconds = seconds;
    alarmTimer->CalendarTime.Minutes = minutes;
    alarmTimer->CalendarTime.Hours = hours;
    alarmTimer->CalendarDate.Date = days;

    //return calendar;
}

void RtcConvertTimerTimeToCalendarTick( TimerTime_t timeCounter, RtcCalendar_t *calendar)
{
    uint16_t seconds = 0;
    uint16_t minutes = 0;
    uint16_t hours = 0;
    uint16_t days = 1;
    uint8_t months = 1; // Start at 1, month 0 does not exist
    uint16_t years = 0;
    //uint16_t century = 0;
    uint32_t timeCounterTemp = timeCounter;

    // Convert milliseconds to RTC format and add to now
    while( timeCounterTemp >= SecondsInLeapYear )
    {
        if( ( years == 0 ) || ( years % 4 ) == 0 )
        {
            timeCounterTemp -= SecondsInLeapYear;
        }
        else
        {
            timeCounterTemp -= SecondsInYear;
        }
        years++;
        if( years == 100 )
        {
            //century = century + 100;
            years = 0;
        }
    }

    if( timeCounterTemp >= SecondsInYear )
    {
        if( ( years == 0 ) || ( years % 4 ) == 0 )
        {
            // Nothing to be done
        }
        else
        {
            timeCounterTemp -= SecondsInYear;
            years++;
        }
    }

    if( ( years == 0 ) || ( years % 4 ) == 0 )
    {
        while( timeCounterTemp >= ( DaysInMonthLeapYear[ months - 1 ] * SecondsInDay ) )
        {
            timeCounterTemp -= DaysInMonthLeapYear[ months - 1 ] * SecondsInDay;
            months++;
        }
    }
    else
    {
        while( timeCounterTemp >= ( DaysInMonth[ months - 1 ] * SecondsInDay ) )
        {
            timeCounterTemp -= DaysInMonth[ months - 1 ] * SecondsInDay;
            months++;
        }
    }

    // Convert milliseconds to RTC format and add to now
    while( timeCounterTemp >= SecondsInDay )
    {
        timeCounterTemp -= SecondsInDay;
        days++;
    }

    // Calculate hours
    while( timeCounterTemp >= SecondsInHour )
    {
        timeCounterTemp -= SecondsInHour;
        hours++;
    }

    // Calculate minutes
    while( timeCounterTemp >= SecondsInMinute )
    {
        timeCounterTemp -= SecondsInMinute;
        minutes++;
    }

    // Calculate seconds
    seconds = timeCounterTemp;

    calendar->CalendarTime.Seconds = seconds;
    calendar->CalendarTime.Minutes = minutes;
    calendar->CalendarTime.Hours = hours;
    calendar->CalendarDate.Date = days;
    calendar->CalendarDate.Month = months;
    calendar->CalendarDate.Year = years;
    //calendar.CalendarCentury = century;
}

//
//static TimerTime_t RtcConvertCalendarTickToTimerTime( RtcCalendar_t *calendar )
//{
//    TimerTime_t timeCounter = 0;
//    RtcCalendar_t now;
//    double timeCounterTemp = 0.0;
//
//    // Passing a NULL pointer will compute from "now" else,
//    // compute from the given calendar value
//    if( calendar == NULL )
//    {
//        now = RtcGetCalendar( );
//    }
//    else
//    {
//        now = *calendar;
//    }
//
//    // Years (calculation valid up to year 2099)
//    for( int16_t i = 0; i < now.CalendarDate.Year; i++ )
//    {
//        if( ( i == 0 ) || ( i % 4 ) == 0 )
//        {
//            timeCounterTemp += ( double )SecondsInLeapYear;
//        }
//        else
//        {
//            timeCounterTemp += ( double )SecondsInYear;
//        }
//    }
//
//    // Months (calculation valid up to year 2099)*/
//    if( ( now.CalendarDate.Year == 0 ) || ( now.CalendarDate.Year % 4 ) == 0 )
//    {
//        for( uint8_t i = 0; i < ( now.CalendarDate.Month - 1 ); i++ )
//        {
//            timeCounterTemp += ( double )( DaysInMonthLeapYear[i] * SecondsInDay );
//        }
//    }
//    else
//    {
//        for( uint8_t i = 0;  i < ( now.CalendarDate.Month - 1 ); i++ )
//        {
//            timeCounterTemp += ( double )( DaysInMonth[i] * SecondsInDay );
//        }
//    }
//
//    timeCounterTemp += ( double )( ( uint32_t )now.CalendarTime.Seconds +
//                     ( ( uint32_t )now.CalendarTime.Minutes * SecondsInMinute ) +
//                     ( ( uint32_t )now.CalendarTime.Hours * SecondsInHour ) +
//                     ( ( uint32_t )( now.CalendarDate.Date * SecondsInDay ) ) );
//
//    timeCounterTemp = ( double )timeCounterTemp * RTC_ALARM_TICK_DURATION;
//
//    timeCounter = round( timeCounterTemp );
//    return ( timeCounter );
//}
//
//static void RtcCheckCalendarRollOver( uint8_t year )
//{
//    if( year == 99 )
//    {
//        CallendarRollOverReady = true;
//    }
//
//    if( ( CallendarRollOverReady == true ) && ( ( year + Century ) == Century ) )
//    {   // Indicate a roll-over of the calendar
//        CallendarRollOverReady = false;
//        Century = Century + 100;
//    }
//}

static void RtcGetCalendar( RtcCalendar_t *calendar )
{
    uint32_t tmpreg;
    uint32_t tmpreg_tr0, tmpreg_dr0, tmpreg_ssr0;
    uint32_t tmpreg_tr1, tmpreg_dr1, tmpreg_ssr1;
    volatile uint32_t counter = 0;

    //RTC_WaitForSynchro();

    do{
        tmpreg_ssr0 = RTC->SSR;
        tmpreg_ssr1 = RTC->SSR;
        tmpreg_tr0 = RTC->TR;
        tmpreg_tr1 = RTC->TR;
        tmpreg_dr0 = RTC->DR;
        tmpreg_dr1 = RTC->DR;
        counter++;
    }while( ( (tmpreg_ssr0 != tmpreg_ssr1) || (tmpreg_tr0 != tmpreg_tr1) || (tmpreg_dr0 != tmpreg_dr1) ) &&
            (counter != SYNCHRO_TIMEOUT) );

    calendar->CalendarTime.Subseconds = (RTC_SUBSEC_MAX - 1) - tmpreg_ssr0;

    tmpreg = (uint32_t)(tmpreg_tr0 & RTC_TR_RESERVED_MASK);
    calendar->CalendarTime.Hours = Bcd2ToByte((uint8_t)((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16));
    calendar->CalendarTime.Minutes = Bcd2ToByte((uint8_t)((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >>8));
    calendar->CalendarTime.Seconds = Bcd2ToByte((uint8_t)(tmpreg & (RTC_TR_ST | RTC_TR_SU)));

    tmpreg = (uint32_t)(tmpreg_dr0 & RTC_DR_RESERVED_MASK);
    calendar->CalendarDate.Year = Bcd2ToByte((uint8_t)((tmpreg & (RTC_DR_YT | RTC_DR_YU)) >> 16));
    calendar->CalendarDate.Month = Bcd2ToByte((uint8_t)((tmpreg & (RTC_DR_MT | RTC_DR_MU)) >> 8));
    calendar->CalendarDate.Date = Bcd2ToByte((uint8_t)(tmpreg & (RTC_DR_DT | RTC_DR_DU)));

//    printf("%d %d %d %d %d %d %d\n",
//           calendar->CalendarDate.Year,
//           calendar->CalendarDate.Month,
//           calendar->CalendarDate.Date,
//           calendar->CalendarTime.Hours,
//           calendar->CalendarTime.Minutes,
//           calendar->CalendarTime.Seconds,
//           calendar->CalendarTime.Subseconds
//           );
    //calendar.CalendarCentury = Century;
    //RtcCheckCalendarRollOver( calendar.CalendarDate.Year );
    //return calendar;
}

void RTC_IRQHandler( void )
{
    /* Check alarm A flag */
    if((RTC->ISR & (RTC_ISR_ALRAF)) == (RTC_ISR_ALRAF)){
        RTC->ISR &=~ RTC_ISR_ALRAF; /* clear flag */
    }
    EXTI->PR = EXTI_PR_PR17; /* clear exti line 17 flag */

    RTC_DisalbeAlarmA();
    RtcRecoverMcuStatus( );
    RtcComputeWakeUpTime( );
    BlockLowPowerDuringTask( false );
    TimerIrqHandler( );
}

TimerTime_t RtcTempCompensation( TimerTime_t period, float temperature )
{
    float k = RTC_TEMP_COEFFICIENT;
    float kDev = RTC_TEMP_DEV_COEFFICIENT;
    float t = RTC_TEMP_TURNOVER;
    float tDev = RTC_TEMP_DEV_TURNOVER;
    float interim = 0.0;
    float ppm = 0.0;

    if( k < 0.0 )
    {
        ppm = ( k - kDev );
    }
    else
    {
        ppm = ( k + kDev );
    }
    interim = ( temperature - ( t - tDev ) );
    ppm *=  interim * interim;

    // Calculate the drift in time
    interim = ( ( float ) period * ppm ) / 1e6;
    // Calculate the resulting time period
    interim += period;
    interim = floor( interim );

    if( interim < 0.0 )
    {
        interim = ( float )period;
    }

    // Calculate the resulting period
    return ( TimerTime_t ) interim;
}

uint32_t RtcGetTick( RtcTickType_t ticktype, RtcCalendar_t *cal )
{
    uint32_t days = 0;
    uint8_t i = 0;
    uint64_t ticks;

    RtcCalendar_t calendar_var;
    RtcCalendar_t *calendar;

    if( cal == NULL ){
        calendar = &calendar_var;
        RtcGetCalendar(calendar);
    }else{
        calendar = cal;
    }

    // years
    for( i = 0; i < calendar->CalendarDate.Year; i++ ){
        if( ( i == 0 ) || ( i % 4 == 0 ) ){
            days += 366;
        }
        else{
            days += 365;
        }
    }

    for( i = 1; i < ( calendar->CalendarDate.Month ); i++ ){
        switch(i){
        case 4:
        case 6:
        case 9:
        case 11:
            days += 30;
            break;
        case 2:
            if( (calendar->CalendarDate.Year % 4) == 0 ){
                days += 29;
            }else{
                days += 28;
            }
            break;
        default:
            days += 31;
            break;
        }
    }

    if(ticktype == SUBSEC){
        ticks = ( (uint64_t)calendar->CalendarTime.Subseconds +
                (uint64_t)( calendar->CalendarTime.Seconds * SubsecsInSecond) +
                (uint64_t)( calendar->CalendarTime.Minutes * SubsecsInMinute ) +
                (uint64_t)( calendar->CalendarTime.Hours * SubsecsInHour ) +
                (uint64_t)( ( (days+ calendar->CalendarDate.Date-1) * SubsecsInDay ) ) );
        return (uint32_t)ticks;
    }else if(ticktype == SECOND){
        return ( (uint32_t )calendar->CalendarTime.Seconds +
                ( ( uint32_t )calendar->CalendarTime.Minutes * 60 ) +
                ( ( uint32_t )calendar->CalendarTime.Hours * (60*60) ) +
                ( ( uint32_t )( (days+ calendar->CalendarDate.Date-1) * (60*60*24) ) ) );
    }else{
        ticks = ( (uint64_t)calendar->CalendarTime.Subseconds +
                (uint64_t)( calendar->CalendarTime.Seconds * SubsecsInSecond) +
                (uint64_t)( calendar->CalendarTime.Minutes * SubsecsInMinute ) +
                (uint64_t)( calendar->CalendarTime.Hours * SubsecsInHour ) +
                (uint64_t)( ( (days+ calendar->CalendarDate.Date-1) * SubsecsInDay ) ) );

        ticks = (uint32_t)round(RTC_ALARM_TICK_DURATION*ticks);

        return (uint32_t)ticks;
    }
}

void RtcDelayMs ( uint32_t delay )
{
    uint32_t tick, curtick, diff;

    // delay to ticks;
    delay = (uint32_t)(delay * RTC_ALARM_TICK_PER_MS);

    tick = RtcGetTick(SUBSEC, NULL);
    do{
        curtick = RtcGetTick(SUBSEC, NULL);
        diff = curtick - tick;
    }while(diff < delay);
}

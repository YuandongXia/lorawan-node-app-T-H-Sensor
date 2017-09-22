/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

  This program and the accompanying materials are made available under the
  terms of the Eclipse Public License v1.0 which accompanies this
  distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

Brief: print banner

Author: YF
*/
#include <stdio.h>
#include "banner.h"

const char risinghf[5][81] = {
    "    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/",
    "   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/     ",
    "  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/  ",
    " _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/       ",
    "_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/        "
};

#ifdef BANNER
void banner(void)
{
    int i;
    printf("\r\n");
    for(i=0; i<5; i++){
        printf("%s\r\n", risinghf[i]);
    }
    printf("\r\n");
}
#endif


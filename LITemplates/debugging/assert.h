#pragma once
#include "litlogger.h"
#define Assertf(cond) do{if(!(cond)) {leFatalf("[ASSERTION] Assertion Failed for condition %s", #cond);}}while(false);
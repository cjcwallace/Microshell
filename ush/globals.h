/* Cameron Wallace
 * April 29, 2020
 * CSCI 347 Spring
 * Assignment 3
 */

#pragma once

#ifdef DEFINE_GLOBALS

#define GLOBAL_VAR(type, name, init) extern type name ; type name = init

#else

#define GLOBAL_VAR(type, name, init) extern type name

#endif

/* Actual Global Vars */

GLOBAL_VAR (int, gargc, 0);

GLOBAL_VAR (char **, gargv, NULL);

GLOBAL_VAR (int, gshift, 0);

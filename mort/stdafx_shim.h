// What MORTDecoder.cpp needed from Windows' StdAfx.h
#pragma once
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define MessageBox(a, text, title, c) fprintf(stderr, "mort: %s\n", text)
#define __int64 long long

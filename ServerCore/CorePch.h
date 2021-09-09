#pragma once

#include "CoreMacro.h"
#include "Container.h"
#include "CorePch.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"
#include "Allocator.h"


#include <Windows.h>
#include <iostream>
using namespace std;


#include <winsock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

#include "Lock.h"

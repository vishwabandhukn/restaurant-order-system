@echo off
echo Compiling Restaurant System...
set GPP=C:\msys64\ucrt64\bin\g++.exe
"%GPP%" main.cpp Database/db.cpp Services/OrderService.cpp Services/BillingService.cpp -o RestaurantSystem.exe -lsqlite3 -std=c++14 -I"C:\msys64\ucrt64\include" -L"C:\msys64\ucrt64\lib"
if %errorlevel% neq 0 (
    echo Compilation failed!
) else (
    echo Compilation successful! Run: .\RestaurantSystem.exe
)

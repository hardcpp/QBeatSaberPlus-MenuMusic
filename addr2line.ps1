param($p1, $p2)
$NDKPath = $env:ANDROID_NDK_HOME

if ($p1 -and $p2)
{
    & $NDKPath\toolchains\llvm\prebuilt\windows-x86_64\bin\llvm-addr2line.exe -e .\build\debug\$p1 $p2
}
else
{
    if ($p1)
    {
        & $NDKPath\toolchains\llvm\prebuilt\windows-x86_64\bin\llvm-addr2line.exe -e .\build\debug\libqbeatsaberplus-menumusic.so $p1
    }
    else
    {
        echo give at least 1 argument
    }
}
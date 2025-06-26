# --- Configuration ---
$PSScriptRoot = Split-Path -Parent -Path $MyInvocation.MyCommand.Definition

# Compilers and Tools
$NASM = "nasm"
$GCC = "i686-elf-gcc"
$LD = "i686-elf-ld"
$HOST_CC = "gcc" # Assumes gcc is in the WSL/system path

# Directories
$BuildDir = "$PSScriptRoot\build"
$IsoDir = "$PSScriptRoot\iso"
$IsoRoot = "$IsoDir\isodir"
$ToolsDir = "$PSScriptRoot\tools"

# Files
$KernelBinary = "$IsoRoot\boot\kernel.bin"
$IsoFile = "$IsoDir\bos.iso"
$LinkerScript = "$PSScriptRoot\link.ld"

# Compiler Flags
$NasmFlags = "-f elf32"
$GccFlags = "-ffreestanding -O2 -Wall -Wextra -I`"$PSScriptRoot\krnl`" -c"

# --- Clean Up ---
Write-Host "--- Cleaning up previous build ---" -ForegroundColor Yellow
if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
}
if (Test-Path $IsoDir) {
    Remove-Item -Recurse -Force $IsoDir
}
New-Item -ItemType Directory -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Path $IsoDir | Out-Null
New-Item -ItemType Directory -Path $IsoRoot | Out-Null
New-Item -ItemType Directory -Path "$IsoRoot\boot" | Out-Null
New-Item -ItemType Directory -Path "$IsoRoot\boot\grub" | Out-Null

# --- Build mkinitrd Tool ---
Write-Host "--- Compiling mkinitrd host tool ---" -ForegroundColor Cyan
$mkinitrd_src = "$ToolsDir\mkinitrd.c"
$mkinitrd_exe = "$ToolsDir\mkinitrd.exe"
$process = Start-Process -FilePath $HOST_CC -ArgumentList "-o `"$mkinitrd_exe`" `"$mkinitrd_src`"" -NoNewWindow -PassThru -Wait
if ($process.ExitCode -ne 0) {
    Write-Host "FATAL: Compilation of mkinitrd.c failed!" -ForegroundColor Red
    exit 1
}

# --- Create initrd ---
Write-Host "--- Creating Initial RAM Disk (initrd) ---" -ForegroundColor Cyan
$initrd_contents = "$PSScriptRoot\initrd_contents"
$initrd_img = "$IsoRoot\boot\initrd.img"
if (-not (Test-Path $initrd_contents)) {
    New-Item -ItemType Directory -Path $initrd_contents | Out-Null
    Set-Content -Path "$initrd_contents\hello.txt" -Value "Hello from the initrd!"
}

# Build the argument list for mkinitrd.exe
$argumentList = @("`"$initrd_img`"")
Get-ChildItem -Path $initrd_contents -File | ForEach-Object {
    $argumentList += "`"$($_.FullName)`""
    $argumentList += "`"$($_.Name)`""
}
$process = Start-Process -FilePath $mkinitrd_exe -ArgumentList $argumentList -NoNewWindow -PassThru -Wait
if ($process.ExitCode -ne 0) {
    Write-Host "FATAL: Failed to create initrd.img" -ForegroundColor Red
    exit 1
}

# --- Compile Kernel ---
Write-Host "--- Compiling Kernel ---" -ForegroundColor Cyan

# Source Files
$asm_sources = Get-ChildItem -Path "$PSScriptRoot" -Recurse -Include *.asm | Where-Object { $_.FullName -notlike "*\boot\*" }
$asm_sources += Get-ChildItem -Path "$PSScriptRoot\boot" -Include boot.asm, interrupts.asm
$c_sources = Get-ChildItem -Path "$PSScriptRoot\krnl" -Recurse -Include *.c

$object_files = @()

# Compile Assembly
foreach ($source in $asm_sources) {
    $object_file = "$BuildDir\$($source.Name).o"
    $object_files += $object_file
    Write-Host "Assembling $($source.Name)..."
    $process = Start-Process -FilePath $NASM -ArgumentList "$NasmFlags `"$($source.FullName)`" -o `"$object_file`"" -NoNewWindow -PassThru -Wait
    if ($process.ExitCode -ne 0) {
        Write-Host "FATAL: Assembly of $($source.Name) failed!" -ForegroundColor Red
        exit 1
    }
}

# Compile C
foreach ($source in $c_sources) {
    $object_file = "$BuildDir\$($source.Name).o"
    $object_files += $object_file
    Write-Host "Compiling $($source.Name)..."
    $process = Start-Process -FilePath $GCC -ArgumentList "$GccFlags `"$($source.FullName)`" -o `"$object_file`"" -NoNewWindow -PassThru -Wait
    if ($process.ExitCode -ne 0) {
        Write-Host "FATAL: Compilation of $($source.Name) failed!" -ForegroundColor Red
        exit 1
    }
}

# --- Link Kernel ---
Write-Host "--- Linking Kernel ---" -ForegroundColor Cyan
$object_list = $object_files -join " "
$process = Start-Process -FilePath $LD -ArgumentList "-T `"$LinkerScript`" -o `"$KernelBinary`" $object_list" -NoNewWindow -PassThru -Wait
if ($process.ExitCode -ne 0) {
    Write-Host "FATAL: Linking failed!" -ForegroundColor Red
    exit 1
}

# --- Create Bootable ISO ---
Write-Host "--- Creating Bootable ISO ---" -ForegroundColor Cyan
Set-Content -Path "$IsoRoot\boot\grub\grub.cfg" -Value @"
menuentry `"BOS`" {
    multiboot /boot/kernel.bin
    module /boot/initrd.img
}
"@

$wslIsoRoot = (wsl wslpath -a $IsoRoot).Trim()
$wslIsoFile = (wsl wslpath -a $IsoFile).Trim()

$process = Start-Process -FilePath "wsl" -ArgumentList "grub-mkrescue -o `"$wslIsoFile`" `"$wslIsoRoot`"" -NoNewWindow -PassThru -Wait
if ($process.ExitCode -ne 0) {
    Write-Host "FATAL: ISO creation failed!" -ForegroundColor Red
    exit 1
}

Write-Host "--- Build Successful! ---" -ForegroundColor Green
Write-Host "ISO created at: $IsoFile"

# --- Run QEMU ---
Write-Host "--- Starting QEMU ---" -ForegroundColor Yellow
$wslIsoFileForQemu = (wsl wslpath -w $IsoFile).Trim()
Start-Process -FilePath "qemu-system-i386" -ArgumentList "-cdrom `"$wslIsoFileForQemu`""

Write-Host "`n--- Script finished ---" -ForegroundColor Green
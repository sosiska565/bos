# --- НАСТРОЙКИ ---
# Пути к вашим компиляторам Windows
$NasmCompiler = "nasm.exe"
$GccCompiler  = "i686-elf-gcc.exe"
$Linker       = "i686-elf-ld.exe"

# Флаги для компиляторов
$GccFlags = "-m32 -c -ffreestanding -Wall -Wextra -Ikrnl"
$NasmFlags = "-f elf32"

# Имена файлов и папок
$BuildDir = "build"
$KernelBinary = "bos.bin"
$LinkerScript = "link.ld"

# --- ОСНОВНАЯ ЛОГИКА ---

function Write-HostColored($Message, $Color) {
    Write-Host $Message -ForegroundColor $Color
}

# 1. Очистка и подготовка
Write-HostColored "--- Cleaning up previous build ---" "Yellow"
if (Test-Path $BuildDir) {
    Remove-Item -Path "$BuildDir\*.o" -ErrorAction SilentlyContinue
} else {
    New-Item -Path $BuildDir -ItemType Directory | Out-Null
}
Remove-Item -Path $KernelBinary -ErrorAction SilentlyContinue

# 2. Компиляция ассемблерного файла (происходит в Windows)
Write-HostColored "--- Assembling bootloader ---" "Cyan"
$AsmSource = "boot\boot.asm"
$AsmObject = "$BuildDir\boot.o"
Invoke-Expression "$NasmCompiler $NasmFlags $AsmSource -o $AsmObject"
if ($LASTEXITCODE -ne 0) { Write-HostColored "Assembling failed!" "Red"; exit 1 }

# 3. Компиляция C-файлов (происходит в Windows)
Write-HostColored "--- Compiling C source files ---" "Cyan"
$c_sources = @("krnl\header.c", "krnl\vga\vga.c", "krnl\keyboard\keyboard.c", "krnl\string\string.c")
$object_files = @("$BuildDir\boot.o")

foreach ($source_file in $c_sources) {
    $object_name = [System.IO.Path]::GetFileNameWithoutExtension($source_file)
    $object_file = "$BuildDir\$object_name.o"
    $object_files += $object_file
    Invoke-Expression "$GccCompiler $GccFlags $source_file -o $object_file"
    if ($LASTEXITCODE -ne 0) { Write-HostColored "Compilation of $source_file failed!" "Red"; exit 1 }
}

# 4. Линковка (происходит в Windows)
Write-HostColored "--- Linking all object files ---" "Cyan"
$AllObjectsString = $object_files -join " "
Invoke-Expression "$Linker -T $LinkerScript -o $KernelBinary $AllObjectsString"
if ($LASTEXITCODE -ne 0) { Write-HostColored "Linking failed!" "Red"; exit 1 }

Write-HostColored "`nBuild successful! Kernel created at: (Get-Location)\$KernelBinary" "Green"

# --- ЗАПУСК QEMU через WSL (ИСПРАВЛЕННАЯ ЧАСТЬ) ---
Write-HostColored "--- Launching QEMU via WSL ---" "Yellow"

# Получаем полный путь к нашему ядру в формате Windows
$windowsKernelPath = (Resolve-Path -Path $KernelBinary).Path

# Используем утилиту 'wsl wslpath', чтобы конвертировать путь Windows в путь WSL
# Например, "C:\Users\me\bos.bin" превратится в "/mnt/c/Users/me/bos.bin"
$wslKernelPath = wsl wslpath -u $windowsKernelPath

Write-Host "Windows path: $windowsKernelPath"
Write-Host "WSL path: $wslKernelPath"

# Формируем и выполняем команду для запуска QEMU внутри WSL
$QemuCommand = "wsl qemu-system-i386 -kernel bos.bin"
Write-Host "Executing: $QemuCommand"
Invoke-Expression $QemuCommand

Write-HostColored "`n--- Script finished ---" "Green"
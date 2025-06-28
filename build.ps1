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
$IsoBinary = "bos.iso"
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
Remove-Item -Path $KernelBinary, $IsoBinary -ErrorAction SilentlyContinue


# 2. КОМПИЛЯЦИЯ ИСХОДНИКОВ
Write-HostColored "--- Compiling all source files ---" "Cyan"

# Список всех ассемблерных файлов
$asm_sources = @(
    "boot\boot.asm",
    "boot\modules\interrupts.asm"
)

# Список всех C-файлов
$c_sources = @(
    "krnl\header.c",
    "krnl\vga\vga.c",
    "krnl\keyboard\keyboard.c",
    "krnl\string\string.c",
    "krnl\interrupts\idt.c",
    "krnl\interrupts\pic.c",
    "krnl\memory\memory.c",
    "krnl\drivers\pci.c",
    "krnl\drivers\io.c",
    "krnl\timer\timer.c",
    "krnl\shell\shell.c"
)

# Создаем пустой список для всех объектных файлов, который будем пополнять
$object_files = @()

# Цикл для компиляции ассемблерных файлов
Write-Host "Assembling .asm files..."
foreach ($source_file in $asm_sources) {
    $object_name = [System.IO.Path]::GetFileNameWithoutExtension($source_file)
    $object_file = "$BuildDir\$object_name.o"
    $object_files += $object_file
    
    Invoke-Expression "$NasmCompiler $NasmFlags $source_file -o $object_file"
    if ($LASTEXITCODE -ne 0) { Write-HostColored "Assembling of $source_file failed!" "Red"; exit 1 }
}

# Цикл для компиляции C-файлов
Write-Host "Compiling .c files..."
foreach ($source_file in $c_sources) {
    $object_name = [System.IO.Path]::GetFileNameWithoutExtension($source_file)
    $object_file = "$BuildDir\$object_name.o"
    $object_files += $object_file

    Invoke-Expression "$GccCompiler $GccFlags $source_file -o $object_file"
    if ($LASTEXITCODE -ne 0) { Write-HostColored "Compilation of $source_file failed!" "Red"; exit 1 }
}


# 3. Линковка (происходит в Windows)
Write-HostColored "--- Linking all object files ---" "Cyan"
$AllObjectsString = $object_files -join " "
Invoke-Expression "$Linker -T $LinkerScript -o $KernelBinary $AllObjectsString"
if ($LASTEXITCODE -ne 0) { Write-HostColored "Linking failed!" "Red"; exit 1 }

Write-HostColored "`nBuild successful! Kernel created at: $KernelBinary" "Green"


# 4. СОЗДАНИЕ ISO-ОБРАЗА
Write-HostColored "--- Creating bootable ISO image ---" "Yellow"

$isoDir = "iso_root"
$isoBootDir = "$isoDir\boot"
$isoGrubDir = "$isoBootDir\grub"

if (Test-Path $isoDir) { Remove-Item -Recurse -Force $isoDir }
New-Item -ItemType Directory -Path $isoDir, $isoBootDir, $isoGrubDir | Out-Null

$grubCfgContent = "set timeout=0`nset default=0`nmenuentry `"BOS`" {`n    multiboot /boot/bos.bin`n    boot`n}"
Set-Content -Path "$isoGrubDir\grub.cfg" -Value $grubCfgContent
Copy-Item -Path $KernelBinary -Destination $isoBootDir

$wslIsoDir = wsl wslpath -a $isoDir
$IsoCommand = "wsl grub-mkrescue -o $IsoBinary $wslIsoDir"
Write-Host "Executing: $IsoCommand"
Invoke-Expression $IsoCommand
if ($LASTEXITCODE -ne 0) { Write-HostColored "ISO creation failed!" "Red"; exit 1 }

Write-HostColored "`nISO image '$IsoBinary' created successfully!" "Green"


# 5. ЗАПУСК QEMU С ISO
Write-HostColored "--- Launching QEMU with ISO ---" "Yellow"
$QemuCommand = "wsl qemu-system-i386 -kernel bos.bin"
Write-Host "Executing: $QemuCommand"
Invoke-Expression $QemuCommand

Write-HostColored "`n--- Script finished ---" "Green"
# Get the directory where the script is located
$ScriptRoot = $PSScriptRoot

# --- НАСТРОЙКИ ---
# Пути к вашим компиляторам Windows
$NasmCompiler = "nasm.exe"
$GccCompiler  = "i686-elf-gcc.exe"
$Linker       = "i686-elf-ld.exe"

# Флаги для компиляторов
$GccFlags = "-m32 -c -ffreestanding -Wall -Wextra -I`"$ScriptRoot\krnl`""
$NasmFlags = "-f elf32"

# --- Create initrd ---
Write-Host "Creating initrd..."

# Define paths for initrd
$InitrdContentsDir = Join-Path $ScriptRoot "initrd_contents"
$InitrdToolSrc = Join-Path $ScriptRoot "tools\mkinitrd.c"
$InitrdToolExe = Join-Path $ScriptRoot "tools\mkinitrd.exe"
$InitrdImage = Join-Path $ScriptRoot "initrd.img"
$File1Path = Join-Path $InitrdContentsDir "file1.txt"
$File2Path = Join-Path $InitrdContentsDir "file2.txt"

# Create a directory for files that will go into the initrd
New-Item -ItemType Directory -Force -Path $InitrdContentsDir
Set-Content -Path $File1Path -Value "This is file1."
Set-Content -Path $File2Path -Value "This is file2."

# Compile the tool to create the initrd
Invoke-Expression "$GccCompiler `"$InitrdToolSrc`" -o `"$InitrdToolExe`""
if ($LASTEXITCODE -ne 0) { Write-HostColored "Compilation of mkinitrd.c failed!" "Red"; exit 1 }

# Run the tool to create initrd.img
& $InitrdToolExe "$InitrdImage" "$File1Path" "file1.txt" "$File2Path" "file2.txt"
if ($LASTEXITCODE -ne 0) { Write-HostColored "Creation of initrd.img failed!" "Red"; exit 1 }

# --- End create initrd ---

# Имена файлов и папок
$BuildDir = Join-Path $ScriptRoot "build"
$KernelBinary = Join-Path $ScriptRoot "bos.bin"
$IsoBinary = Join-Path $ScriptRoot "bos.iso"
$LinkerScript = Join-Path $ScriptRoot "link.ld"

# --- ОСНОВНАЯ ЛОГИКА ---

function Write-HostColored($Message, $Color) {
    Write-Host $Message -ForegroundColor $Color
}

# 1. Очистка и подготовка
Write-HostColored "--- Cleaning up previous build ---" "Yellow"
if (Test-Path $BuildDir) {
    Remove-Item -Path (Join-Path $BuildDir "*.o") -ErrorAction SilentlyContinue
} else {
    New-Item -Path $BuildDir -ItemType Directory | Out-Null
}
Remove-Item -Path $KernelBinary, $IsoBinary -ErrorAction SilentlyContinue


# 2. КОМПИЛЯЦИЯ ИСХОДНИКОВ
Write-HostColored "--- Compiling all source files ---" "Cyan"

# Список всех ассемблерных файлов
$asm_sources = @(
    (Join-Path $ScriptRoot "boot\boot.asm"),
    (Join-Path $ScriptRoot "boot\modules\interrupts.asm"),
    (Join-Path $ScriptRoot "krnl\process\switch.asm"),
    (Join-Path $ScriptRoot "krnl\process\read_eip.asm"),
    (Join-Path $ScriptRoot "krnl\gdt\gdt.asm")
)

# Список всех C-файлов
$c_sources = @(
    (Join-Path $ScriptRoot "krnl\header.c"),
    (Join-Path $ScriptRoot "krnl\vga\vga.c"),
    (Join-Path $ScriptRoot "krnl\keyboard\keyboard.c"),
    (Join-Path $ScriptRoot "krnl\string\string.c"),
    (Join-Path $ScriptRoot "krnl\interrupts\idt.c"),
    (Join-Path $ScriptRoot "krnl\interrupts\pic.c"),
    (Join-Path $ScriptRoot "krnl\memory\memory.c"),
    (Join-Path $ScriptRoot "krnl\memory\pmm.c"),
    (Join-Path $ScriptRoot "krnl\memory\vmm.c"),
    (Join-Path $ScriptRoot "krnl\memory\heap.c"),
    (Join-Path $ScriptRoot "krnl\utils\utils.c"),
    (Join-Path $ScriptRoot "krnl\utils\ordered_array.c"),
    (Join-Path $ScriptRoot "krnl\drivers\pci.c"),
    (Join-Path $ScriptRoot "krnl\drivers\io.c"),
    (Join-Path $ScriptRoot "krnl\timer\timer.c"),
    (Join-Path $ScriptRoot "krnl\process\task.c"),
    (Join-Path $ScriptRoot "krnl\shell\shell.c"),
    (Join-Path $ScriptRoot "krnl\fs\vfs.c"),
    (Join-Path $ScriptRoot "krnl\fs\initrd.c"),
    (Join-Path $ScriptRoot "krnl\gdt\gdt.c")
)

# Создаем пустой список для всех объектных файлов, который будем пополнять
$object_files = @()

# Цикл для компиляции ассемблерных файлов
Write-Host "Assembling .asm files..."
foreach ($source_file in $asm_sources) {
    $object_name = [System.IO.Path]::GetFileNameWithoutExtension($source_file)
    $object_file = Join-Path $BuildDir "$object_name.o"
    $object_files += "`"$object_file`""
    
    Invoke-Expression "$NasmCompiler $NasmFlags `"$source_file`" -o `"$object_file`""
    if ($LASTEXITCODE -ne 0) { Write-HostColored "Assembling of $source_file failed!" "Red"; exit 1 }
}

# Цикл для компиляции C-файлов
Write-Host "Compiling .c files..."
foreach ($source_file in $c_sources) {
    $object_name = [System.IO.Path]::GetFileNameWithoutExtension($source_file)
    $object_file = Join-Path $BuildDir "$object_name.o"
    $object_files += "`"$object_file`""

    Invoke-Expression "$GccCompiler $GccFlags `"$source_file`" -o `"$object_file`""
    if ($LASTEXITCODE -ne 0) { Write-HostColored "Compilation of $source_file failed!" "Red"; exit 1 }
}


# 3. Линковка (происходит в Windows)
Write-HostColored "--- Linking all object files ---" "Cyan"
$AllObjectsString = $object_files -join " "
Invoke-Expression "$Linker -T `"$LinkerScript`" -o `"$KernelBinary`" $AllObjectsString"
if ($LASTEXITCODE -ne 0) { Write-HostColored "Linking failed!" "Red"; exit 1 }

Write-HostColored "`nBuild successful! Kernel created at: $KernelBinary" "Green"


# 4. СОЗДАНИЕ ISO-ОБРАЗА
Write-HostColored "--- Creating bootable ISO image ---" "Yellow"

$isoDir = Join-Path $ScriptRoot "iso_root"
$isoBootDir = Join-Path $isoDir "boot"
$isoGrubDir = Join-Path $isoBootDir "grub"

if (Test-Path $isoDir) { Remove-Item -Recurse -Force $isoDir }
New-Item -ItemType Directory -Path $isoDir, $isoBootDir, $isoGrubDir | Out-Null

$grubCfgContent = "set timeout=0`nset default=0`nmenuentry `"BOS`" {`n    multiboot /boot/bos.bin`n    module /boot/initrd.img`n    boot`n}"
Set-Content -Path (Join-Path $isoGrubDir "grub.cfg") -Value $grubCfgContent
Copy-Item -Path $KernelBinary -Destination $isoBootDir
Copy-Item -Path $InitrdImage -Destination $isoBootDir

$wslIsoDir = (wsl wslpath -a $isoDir).Trim()
$wslIsoBinary = (wsl wslpath -a $IsoBinary).Trim()
$IsoCommand = "wsl grub-mkrescue -o `"$wslIsoBinary`" `"$wslIsoDir`""
Write-Host "Executing: $IsoCommand"
Invoke-Expression $IsoCommand
if ($LASTEXITCODE -ne 0) { Write-HostColored "ISO creation failed!" "Red"; exit 1 }

Write-HostColored "`nISO image '$IsoBinary' created successfully!" "Green"


# 5. ЗАПУСК QEMU С ISO
Write-HostColored "--- Launching QEMU with ISO ---" "Yellow"
$wslKernelPath = (wsl wslpath -a $KernelBinary).Trim()
$QemuCommand = "wsl qemu-system-i386 -kernel `"$wslKernelPath`""
Write-Host "Executing: $QemuCommand"
Invoke-Expression $QemuCommand

Write-HostColored "`n--- Script finished ---" "Green"
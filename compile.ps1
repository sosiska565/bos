cd C:\\Users\\egor\\Desktop\\basic_operation_system

#Bootloader
nasm -f bin C:\Users\egor\Desktop\basic_operation_system\bootloader.asm -o C:\Users\egor\Desktop\basic_operation_system\os\bootloader.bin
#ASM kernel
nasm -f bin C:\Users\egor\Desktop\basic_operation_system\_kernelASM.asm -o os\_kernelASM.bin

wsl -e sh -c "cat os/bootloader.bin os/_kernelASM.bin > os/x86/setup.bin"

$file = "C:\Users\egor\Desktop\basic_operation_system\os\x86\setup.bin"
$targetSize = 1048576  # 1MB
$currentSize = (Get-Item $file).Length
$padding = $targetSize - $currentSize
if ($padding -gt 0) {
    $stream = [System.IO.File]::OpenWrite($file)
    $stream.Seek(0, [System.IO.SeekOrigin]::End) | Out-Null
    $nulls = New-Object byte[] $padding
    $stream.Write($nulls, 0, $padding)
    $stream.Close()
}

& "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" convertfromraw C:\Users\egor\Desktop\basic_operation_system\os\x86\setup.bin C:\Users\egor\Desktop\basic_operation_system\os\x86\setup.vdi --format VDI
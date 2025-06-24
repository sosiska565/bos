

$file = "C:\Users\egor\Desktop\basic_operation_system\os\x86\disk.img"
$targetSize = 1048576
$currentSize = (Get-Item $file).Length
$padding = $targetSize - $currentSize
if ($padding -gt 0) {
    $nulls = [byte[]]::new($padding)
    Add-Content -Path $file -Value $nulls -Encoding Byte
}

& "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" convertfromraw C:\Users\egor\Desktop\basic_operation_system\os\x86\disk.img C:\Users\egor\Desktop\basic_operation_system\os\x86\disk.vdi --format VDI
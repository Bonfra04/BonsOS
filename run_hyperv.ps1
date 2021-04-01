bash -c "make clean; make"
echo Generating image...
bash -c "./create_image.sh >& /dev/null"
echo Running

del BonsOS.vhd
copy BonsOS.img BonsOS.vhd
tools\VHDTool.exe /convert BonsOS.vhd

Start-Process -Verb RunAs powershell.exe -Args "-executionpolicy bypass -command Set-Location \`"$PWD\`"; Start-VM BonsOS; vmconnect.exe $Env:computername BonsOS"
echo "Installing SambaCompiler to system"
gcc samba.c -o samba # Compiling samba
gcc base.c -o libbase.so -shared -fPIC # Compiling base Plugin (needed in line 66 in samba.c)
./samba --install_SC # Installing SambaCompiler to the system
clear
echo "SambaCompiler installed successfully"
sleep 3
clear
./samba --help




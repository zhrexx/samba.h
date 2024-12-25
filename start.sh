echo "Installing SambaCompiler to system"

if [ -f "samba" ]; then
    if [ "$1" == "--self" ]; then
        echo "Because of --self running using SambaCompiler $(samba)"
        samba rebuild
        exit 0
    fi
fi

gcc samba.c -o samba # Compiling samba
gcc base.c -o libbase.so -shared -fPIC # Compiling base Plugin (needed in line 66 in samba.c)
./samba --install_SC # Installing SambaCompiler to the system
clear
echo "SambaCompiler installed successfully"
sleep 2
clear
./samba --help

**To re-create the demo for the name-change problem of MP1**
```
  make clean
  make
  sh copykernel.sh
  bochs -f bochsrc.bxrc
```
**To re-create the demo for gdb running with boch**
```
  make -f debug_makefile clean
  make -f debug_makefile
  sh debug_copykernel.sh
  bochs -f debug_bochsrc.bxrc
```
  Open a new terminal in this directory
  ```
    gdb
    b main()
    c
    c
```

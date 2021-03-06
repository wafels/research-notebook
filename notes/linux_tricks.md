## Linux Tricks

* How to redirect stdout and stderr output messages to a file?

```
$ command > file.txt 2>&1
```

* PPM to YUV

```
$ cat images/*.ppm | ppmtoy4m > video.yuv
```

* YUV to PPM

```
$ y4mtoppm < video_input.y4m | pnmsplit - "%d.ppm"
```

* How to invert screen colors in GNU/Linux distros
 
```
xcalib -i -a
```

* How to use ```sshfs``` to mount remote file systems over SSH 

```
ssh user@xxx.xxx.xxx.xxx:/remote_dir /local_dir
```

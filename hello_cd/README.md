##  Loading a file from CD

You need [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) in your $PATH to generate a PSX disk image.

### Compile

```bash
make all
```

### Clean directory

```bash
make cleansub
```

### Adding data to the CD

In `isoconfig.xml`, in the data track's `directory_tree` section, use this syntax :

```xml
<file name="HELO.DAT"   type="data" source="theFile.dat"/>
```

See https://github.com/Lameguy64/mkpsxiso/blob/master/examples/example.xml for more details.

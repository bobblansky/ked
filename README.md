# kEd
kEd is a light footprint single buffer editor.

To install clone repo and run


$ glib-compile-resources res/app_resource.xml --target=resources.c --sourcedir=res --generate-source

$ gcc -o ked app.c resources.c `pkg-config --cflags --libs gtk+-3.0'

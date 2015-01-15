
#!/bin/sh

#Note: package generated will be in the last directory of current(not root)

D="$(basename $(pwd))"

i=0
F="$D""_src_""$(date +%Y%m%d)""_""v$i.tar.gz" 

cd ..

while [ -r "$F" ] ; do
	i=$(($i+1))
	F="$D""_src_""$(date +%Y%m%d)""_""v$i.tar.gz" 
done

tar zcvf "$F" "$D"
chmod 777 "$F"
ls -lh "$F"


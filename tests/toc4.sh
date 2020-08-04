:
# Test if hxtoc treats some HTML5 elements correctly

trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<EOF
<!DOCTYPE html>
<html>
<head lang="en">
<meta charset="UTF-8">
<title>Title</title>
</head>
<body>
<p>
<video controls><source src="gillette_media/powwow_highway.mp4" type="video/mp4">Video</video>
<em>Powwow Highway: Gillette scene</em>
</p>
</body>
</html>
EOF

cat >$TMP2 <<EOF
<!DOCTYPE html><html><head lang="en"><meta charset="UTF-8"><title>Title</title></head><body><p>
<video controls><source src="gillette_media/powwow_highway.mp4" type="video/mp4">Video</video>
<em>Powwow Highway: Gillette scene</em>
</p></body></html>
EOF

./hxtoc $TMP1 >$TMP3
echo >>$TMP3			# Add newline
diff -u $TMP2 $TMP3

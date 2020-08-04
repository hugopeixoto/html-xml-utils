:
# Test of some HTML5 elements

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
<video controls><source src="gillette/powwow_highway.mp4" type="video/mp4">Video</video>
<em>Powwow Highway: Gillette scene</em>
</p>
</body>
</html>
EOF

cat >$TMP2 <<EOF
<!DOCTYPE html>

<html>
  <head lang=en>
    <meta charset=UTF-8>
    <title>Title</title>

  <body>
    <p> <video controls>
      <source src="gillette/powwow_highway.mp4" type="video/mp4">
      Video</video> <em>Powwow Highway: Gillette scene</em>
EOF

./hxnormalize $TMP1 >$TMP3
diff -u $TMP2 $TMP3

:
# Test of "noscript"

trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<EOF
<!doctype html>
<html lang=en>
<head>
<meta charset=utf-8>
<title>example</title>
</head>
<body>
<p>
text
<noscript>noscript</noscript>                                                                                                                                                
text
</p>
</body>
</html>
EOF

cat >$TMP2 <<EOF
<!DOCTYPE html>

<html lang=en>
  <head>
    <meta charset=utf-8>
    <title>example</title>
  </head>

  <body>
    <p> text <noscript>noscript</noscript> text</p>
  </body>
</html>
EOF

./hxnormalize -e $TMP1 >$TMP3
diff -u $TMP2 $TMP3

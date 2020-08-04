:
# Test of HTML5 element "main"

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
<p>Before the main text.
<section>
<p>Also before the main text. Missing close tag for section.
<main>
<p>Inside the main text. Missing close tag for main.
</html>
EOF

cat >$TMP2 <<EOF
<!DOCTYPE html>

<html>
  <head lang=en>
    <meta charset=UTF-8>
    <title>Title</title>

  <body>
    <p>Before the main text.

    <section>
      <p>Also before the main text. Missing close tag for section.
    </section>

    <main>
      <p>Inside the main text. Missing close tag for main.
    </main>
EOF

./hxnormalize $TMP1 >$TMP3
diff -u $TMP2 $TMP3

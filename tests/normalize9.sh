:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<!DOCTYPE html>

	<figure>
	<p><img src="A1" alt="A2">
	<figcaption><a href="B">bbb</a>
	</figure>
	<figure>
	<p><img src="C" alt="C">
	<figcaption><p><a href="D">ddd</a>
	</figure>
EOF
cat >$TMP2 <<-EOF
	<!DOCTYPE html>

	<html>
	<body>
	<figure>
	<p><img alt="A2" src="A1" /></p>
	
	<figcaption><a href="B">bbb</a></figcaption>
	</figure>
	
	<figure>
	<p><img alt="C" src="C" /></p>
	
	<figcaption>
	<p><a href="D">ddd</a></p>
	</figcaption>
	</figure>
	</body>
	</html>
EOF
./hxnormalize -i 0 -L -x $TMP1 >$TMP3

diff -u $TMP2 $TMP3

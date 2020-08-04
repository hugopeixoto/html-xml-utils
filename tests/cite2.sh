:
trap 'rm $TMP1 $TMP2 $TMP3 $TMP4' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP4=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	%L label1

	%L label2
EOF
cat >$TMP2 <<-EOF
	label1
	label1
	label1
	label2
	label1
EOF
./hxcite -c -a $TMP4 $TMP1 >$TMP3 <<-EOF
	aaa [[label1]] bbb [[label1]] ccc
	aaa [[label1]] bbb [label1] ccc
	aaa {{label1]] bbb {label1} ccc
	aaa <!-- bbb
	[[label2]]
	{{[[label1}}
	aaa {{label2}} bbb
	ccc -->
	aaa[[label1]]bbb
EOF
cmp -s $TMP2 $TMP4

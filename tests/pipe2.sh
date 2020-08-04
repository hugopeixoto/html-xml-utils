:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxpipe >$TMP1 <<-EOF
	<abc foo1=bar1 foo2="bar2 bar2">
	text1
	<def/>
	<_foo>text2</_foo>
	text3
	</abc>
EOF
cat >$TMP2 <<-EOF
Afoo1 CDATA bar1
Afoo2 CDATA bar2 bar2
(abc
-\ntext1\n
|def
-\n
(_foo
-text2
)_foo
-\ntext3\n
)abc
-\n
EOF

cmp -s $TMP1 $TMP2

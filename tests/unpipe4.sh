:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxunpipe -b >$TMP1 <<-EOF
	-<>&"'\n
EOF
cat >$TMP2 <<-EOF
	&lt;&gt;&amp;&quot;&apos;
EOF

cmp -s $TMP1 $TMP2

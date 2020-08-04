:
trap 'rm $TMP1 $TMP2 $TMP4' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP4=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

# Make a Refer database
#
cat >$TMP1 <<-EOF
	%L label1
	%K key-a key-b

	%K key-c key-d
	%L label2
EOF

# The expected auxiliary file
#
cat >$TMP2 <<-EOF
	label1
	label1
	label2
	label2
EOF

# Run hxcite
#
./hxcite -c -a $TMP4 $TMP1 >/dev/null <<-EOF
	Here is a reference that uses the label directly:
	[[label1]]

	Here is a reference that uses a key:
	[[key-a]]

	Here is another one:
	[[key-d]]

	And on that uses a key not expanded:
	{{key-c}}
EOF

# Compare the generated auxiliary file
#
cmp -s $TMP2 $TMP4

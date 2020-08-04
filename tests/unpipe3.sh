:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxunpipe >$TMP1 <<-EOF
	?processing instruction
	-\n
	!root "fpi" si
	-\n
	*comment
	L3
	-\n
	Afoo CDATA bar\012
	Abar IMPLIED
	Aid TOKEN x12
	(x
	-line 1\nline 2\n&#10;\012111
	)x
	|empty
	-\n
	C
EOF

cat >$TMP2 <<-EOF
	<?processing instruction>
	<!DOCTYPE root PUBLIC "fpi" "si">
	<!--comment-->
	<x foo="bar&#10;" id="x12">line 1
	line 2
	&#10;&#10;111</x><empty />
EOF

cmp -s $TMP1 $TMP2

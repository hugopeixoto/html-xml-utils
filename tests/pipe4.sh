:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxpipe >$TMP1 <<-EOF
	<αβγ φω1=βαρ1 φω2="βαρ2 βαρ2">
	text1
	<def/>
	<_φω>text2</_φω>
	text3
	</αβγ>
EOF
cat >$TMP2 <<-EOF
Aφω1 CDATA βαρ1
Aφω2 CDATA βαρ2 βαρ2
(αβγ
-\ntext1\n
|def
-\n
(_φω
-text2
)_φω
-\ntext3\n
)αβγ
-\n
EOF

cmp -s $TMP1 $TMP2

:

trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXX` || exit 1

cat >$TMP1 <<EOF
<doc>
<!-- do not remove this -->
<foo title="Do not print this attribute">
<!-- do not remove this -->
</foo>
<!-- do not remove this -->
</doc>
EOF

cat >$TMP2 <<EOF
<doc>
<!-- do not remove this -->
<foo>
<!-- do not remove this -->
</foo>
<!-- do not remove this -->
</doc>
EOF

./hxremove 'foo::attr(title)' <$TMP1 >$TMP3

cmp -s $TMP2 $TMP3

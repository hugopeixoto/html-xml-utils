:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmpXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmpXXXXXX` || exit 1

./hxremove ':root :not(z)' >$TMP1 <<EOF
<doc>
<a>Remove this element</a>
<b>Remove this element</b>
<z><c>Remove this element</c></z>
<d>Remove this <z>with this inside</z></d>
</doc>
EOF

cat >$TMP2 <<EOF
<doc>


<z></z>

</doc>
EOF

diff -u $TMP2 $TMP1

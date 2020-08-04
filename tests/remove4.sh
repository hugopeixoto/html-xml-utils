:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmpXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmpXXXXXX` || exit 1

./hxremove 'li:not([class])' >$TMP1 <<EOF
<ol>
<li>No class</li>
<li>Also no class</li>
<li class=a>With a class</li>
<li class=b>Also with a class</li>
</ol>
EOF

cat >$TMP2 <<EOF
<ol>


<li class="a">With a class</li>
<li class="b">Also with a class</li>
</ol>
EOF

diff -u $TMP2 $TMP1

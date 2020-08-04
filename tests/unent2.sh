:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxunent -b >$TMP1 <<-EOF
	&lt;&lt&lt &lt
	A&amp;B A&B
	A&#65&#65;&#65 &#65
	&#x41;&#x41&#x41 &#x41
	&#34;&#x22;&#34&#x22--&#34;&#x22
	&&unknown;&unknown &unknown
	&#;&#&#y&#x;&#x &#xg
	&eacute;é&#x00E9;&#x00e9&#233;
EOF
cat >$TMP2 <<-EOF
	&lt;&lt;&lt; &lt;
	A&amp;B A&B
	AAAA A
	AAA A
	&#34;&#x22;&#34;&#x22;--&#34;&#x22;
	&&unknown;&unknown &unknown
	&#;&#&#y&#x;&#x &#xg
	ééééé
EOF

diff -u -a $TMP1 $TMP2

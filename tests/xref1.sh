:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxref >$TMP1 <<-EOF
	<p><dfn>term</dfn> is referenced twice:
	<span class="copy-this">term</span> and <em title="term">here.</em>
EOF

# Add a newline at the end:
echo >>$TMP1

cat >$TMP2 <<-EOF
	<html><body><p><dfn id="term">term</dfn> is referenced twice:
	<a href="#term" class="copy-this">term</a> and <a href="#term"><em title="term">here.</em></a>
	</p></body></html>
EOF

cmp -s $TMP1 $TMP2

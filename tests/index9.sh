:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<title>Document title</title>
	<p><dfn>0-term</dfn>
	<h1>Heading 0</h1>
	<h1 class=no-num>Heading 1</h1>
	<p><dfn>A-term</dfn>
	<h2>Heading 2</h2>
	<p><dfn><em>&lt;M-term&gt;</em></dfn>
	<h3>Heading 3</h3>
	<p><dfn>Z-term</dfn>
	<h1>Index</h1>
	<!-- begin-index -->
	<p>Remove this.
	<!-- end-index -->
EOF

# The echo adds a newline at the end of the file
#
(./hxnum $TMP1 | LC_ALL=C ./hxindex -t -N; echo) >$TMP2

cat >$TMP3 <<EOF
<html><head><title>Document title</title></head><body><p><dfn id="term">0-term</dfn>
</p><h1><span class="secno">1. </span>Heading 0</h1><h1 class="no-num">Heading 1</h1><p><dfn id="a-term">A-term</dfn>
</p><h2><span class="secno">1.1. </span>Heading 2</h2><p><dfn id="ltm-termgt"><em>&lt;M-term&gt;</em></dfn>
</p><h3><span class="secno">1.1.1. </span>Heading 3</h3><p><dfn id="z-term">Z-term</dfn>
</p><h1><span class="secno">2. </span>Index</h1><!--begin-index-->
<ul class="indexlist">
<li>0-term, <a href="#term"><strong>Document title</strong></a>
<li>&lt;M-term&gt;, <a href="#ltm-termgt"><strong><span class="secno">1.1. </span>Heading 2</strong></a>
<li>A-term, <a href="#a-term"><strong>Heading 1</strong></a>
<li>Z-term, <a href="#z-term"><strong><span class="secno">1.1.1. </span>Heading 3</strong></a>
</ul><!--end-index--></body></html>
EOF

diff -u $TMP2 $TMP3

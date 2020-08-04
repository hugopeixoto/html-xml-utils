:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<h1>Heading 0</h1>
	<h1 class=no-num>Heading 1</h1>
	<p><dfn title="term!!A">A-term</dfn>
	<p><span class=index title="term!!Z">Z-term</span>
	<h2>Heading 2</h2>
	<p><dfn title="term!!M"><em>&lt;M-term&gt;</em></dfn>
	<p><span class=index title="term!!A  ">A-term</span>
	<h3>Heading 3</h3>
	<p><dfn title="term!!Z ?">Z-term</dfn>
	<p><span class=index title="term!!M.">M-term</span>
	<p><span class=index>term!!M.</span>
	<h1>Index</h1>
	<!-- begin-index -->
	<p>Remove this.
	<!-- end-index -->
EOF

# The echo adds a newline at the end of the file
#
(./hxnum $TMP1 | LC_ALL=C ./hxindex -t -n -f; echo) >$TMP2

cat >$TMP3 <<EOF
<html><body><h1><span class="secno">1. </span>Heading 0</h1><h1 class="no-num">Heading 1</h1><p><dfn id="a-term">A-term</dfn>
</p><p><span id="z-term" class="index">Z-term</span>
</p><h2><span class="secno">1.1. </span>Heading 2</h2><p><dfn id="ltm-termgt"><em>&lt;M-term&gt;</em></dfn>
</p><p><span id="a-term0" class="index">A-term</span>
</p><h3><span class="secno">1.1.1. </span>Heading 3</h3><p><dfn id="z-term0">Z-term</dfn>
</p><p><span id="m-term" class="index">M-term</span>
</p><p><span id="termm." class="index">term!!M.</span>
</p><h1><span class="secno">2. </span>Index</h1><ul class="indexlist">
<li>term
  <ul>
  <li>A, <a href="#a-term" title="section ??"><strong>??</strong></a>, <a href="#a-term0" title="section 1.1.">1.1.</a>
  <li>M, <a href="#ltm-termgt" title="section 1.1."><strong>1.1.</strong></a>, <a href="#m-term" title="section 1.1.1.">1.1.1.</a>, <a href="#termm." title="section 1.1.1.">1.1.1.</a>
  <li>Z, <a href="#z-term" title="section ??">??</a>, <a href="#z-term0" title="section 1.1.1."><strong>1.1.1.</strong></a>
</ul>
</ul></body></html>
EOF

cmp -s $TMP2 $TMP3

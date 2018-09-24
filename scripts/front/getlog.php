<?php
$output = shell_exec('tac /home/rock64/f5p/log.txt');
$dictionary = array(
    '[1;34m' => '<span style="color:aqua">',
    '[1;31m' => '<span style="color:red">',
    '[1;33m' => '<span style="color:yellow">',
    '[0m'   => '</span>' ,
);
$output_color = str_replace(array_keys($dictionary), $dictionary, $output);

//echo '$output_color'

//echo '<span style="color:blue">dsfdsf</span>';
echo "<pre>$output_color</pre>";

?>


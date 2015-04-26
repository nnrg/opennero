
```

//----------------------------------------------------------------
// (BEGIN REQUIRED CODE)
//----------------------------------------------------------------

global string $_write_obj_sequence_path;

global proc int _write_obj_sequence_callback(string $filename, string $fileType) {
	global string $_write_obj_sequence_path;

	$_write_obj_sequence_path = $filename;
	return 1;
}

proc writeFrame(int $frameNumber, string $filename) {
	global string $_write_obj_sequence_path;
	string $filenameWrittenTo;
	string $filenamePlusPath;

	currentTime($frameNumber);

	$filenamePlusPath = $_write_obj_sequence_path + "/" + $filename;
	$filenameWrittenTo = file("-op", "groups=1;ptgroups=1;materials=1;smoothing=1;normals=1",
		"-typ", "OBJexport", "-pr", "-ea", $filenamePlusPath);

	print("Wrote " + $filenameWrittenTo + "\n");
}

// Note: Assumes 3-digit non-negative frame numbers.
proc writeSequence(int $startFrameNumber, int $endFrameNumber, string $baseFilename) {
	int $i;
	int $local_i;
	int $n1;
	int $n2;
	int $n3;
	string $filename;

	for ($i = $startFrameNumber; $i <= $endFrameNumber; ++$i) {
		$local_i = $i - $startFrameNumber + 1;
		$n1 = $local_i / 100;
		$n2 = ($local_i - ($n1 * 100)) / 10;
		$n3 = $local_i - ($n1 * 100) - ($n2 * 10);
		$filename = $baseFilename + ((string) $n1) + ((string) $n2) + ((string) $n3);
		writeFrame($i, $filename);
	}
}

fileBrowserDialog -m 4 -fc "_write_obj_sequence_callback" -an "Choose destination folder.";

//----------------------------------------------------------------
// (BEGIN USER-MODIFIED CODE)
//----------------------------------------------------------------

// usage: writeSequence(first_frame, last_frame, base_filename);
//
// example:
//   writeSequence(89, 102, "run");
//   writeSequence(201, 215, "stand");
//   ...
//
// (Note: base_filename can have at most 16 characters.)

writeSequence(1, 12, "run");
writeSequence(21, 33, "stand");
writeSequence(41, 57, "turn_r_xc");
writeSequence(61, 77, "turn_l_xc");
writeSequence(81, 97, "turn_r_lx");
writeSequence(101, 117, "turn_l_lx");
writeSequence(121, 137, "turn_r_xxx");
writeSequence(141, 157, "turn_l_xxx");
writeSequence(161, 186, "pick_up");
writeSequence(191, 216, "put_down");
writeSequence(221, 232, "hold_run");
writeSequence(241, 253, "hold_stand");
writeSequence(261, 277, "hold_r_xc");
writeSequence(281, 297, "hold_l_xc");
writeSequence(301, 317, "hold_turn_r_lx");
writeSequence(321, 337, "hold_turn_l_lx");
writeSequence(341, 357, "hold_turn_r_xxx");
writeSequence(361, 377, "hold_turn_l_xxx");
writeSequence(381, 410, "jump");
writeSequence(421, 450, "hold_jump");

```
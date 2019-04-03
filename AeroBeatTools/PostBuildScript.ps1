# move dll files
$asmdir = "ABFiles\Assemblies"
New-Item $asmdir -ItemType Directory -Force | Out-Null
$dlls = Get-Item "*.dll" | ?{ $_ -inotmatch "portaudio_x86.dll" }
ForEach ($dll in $dlls)
{
	Move-Item $dll $asmdir -Force
}
$list = Get-ChildItem | ?{ $_.PSIsContainer }
ForEach ($dir in $list)
{
	try
	{
		[System.Globalization.CultureInfo]::new($dir) | Out-Null
		$dst = [System.IO.Path]::Combine($asmdir, $dir)
		if (Test-Path $dst)
		{
			Remove-Item $dst -Force -Recurse
		}
		Move-Item $dir $asmdir
	} catch { }
}

# remove xml documents
Remove-Item "*.xml"

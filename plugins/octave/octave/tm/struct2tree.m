## Released under the GNU General Public License, see www.gnu.org
## Copyright (C) 2002 by the Free Software Foundation
## Written by Michael Graffam mikegraffam@yahoo.com
##
## Convert an Octave structure to a Scheme tree
## Created: Sept 2002

function tmp=struct2tree(n,name)

	global TMCOLORS;
	global TMCOLIDX;

	# Make sure name is defined 
	if ((nargin<2))
		name="(with \"mode\" \"math\" (big \"triangleup\"))";
	endif

	keycolor=deblank(TMCOLORS(1,:));
	valcolor=deblank(TMCOLORS(TMCOLIDX,:));

	tmp=["(tree (with \"color\" \"", keycolor, "\" ",name,") "];
	for [val, key] = n
		switch (typeinfo(val))
			case ("struct")
				## Itemize the new struct, and switch the key color
				tmp1=struct2tree(val,key);
			otherwise
				tmp1=["(switch (document (with \"color\" \"", keycolor,"\" \"",key,"\")) (tuple (tmarker) (with \"color\" \"", valcolor, "\" " ,obj2scm(val),"))) "];
		endswitch
		tmp=[tmp,tmp1];
	endfor
	tmp=[tmp,")"];
endfunction

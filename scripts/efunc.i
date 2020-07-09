#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#    $RCSfile: efunc.i,v $
#    $Revision: 1.1 $
#    $Date: 2001/07/16 08:42:02 $
#    $Author: rice $
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# emboss function indexing
# based on SRS cfunc.i

$EFUNC_DB=$Library:[EFUNC group:$MISC_LIBS 
  format:$EFUNC_FORMAT maxNameLen:80 iFile:efunc
  files:{
    $LibFile:efunc
  }
  links:{
    $Link:[$EFUNC_DB to:$EDATA_DB  token:ref_data toField:$DF_Alias]
    $Link:[$EFUNC_DB to:$EFUNC_DB  token:ref_func toField:$DF_ID
      fromName:EFUNC_UP toName:EFUNC_DOWN]
  }
]

$EFUNC_FORMAT=$LibFormat:[fileType:$EFUNC_FILE syntax:$EFUNC_SYNTAX 
  printFormat:table3 fields:{
    $Field:[$DF_ALL]
    $Field:[$DF_ID code:id index:id indexToken:id]
    $Field:[$DF_Module code:mod index:str indexToken:mod ]
    $Field:[$DF_Library code:lib index:str indexToken:lib ]
    $Field:[$DF_Type code:typ index:str indexToken:typ ]
    $Field:[$DF_Description code:des index:str indexToken:des ]
    $Field:[$DF_Input code:par index:str indexToken:par ]
    $Field:[$DF_Returns code:ret index:str indexToken:retType]
    $Field:[$DF_Prototype code:proto]
    $Field:[$DF_Body code:body index:str indexToken:btext]
  }
]

$EFUNC_SYNTAX=$Syntax:[file:"SRSSITE:efunc.is" ignore:" \t"]
$EFUNC_FILE=$FileType:[typeName:dat maxline:500]

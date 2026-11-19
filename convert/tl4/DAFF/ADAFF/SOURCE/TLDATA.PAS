{$A+,B-,D+,E+,F+,G-,I-,L+,N+,O-,R-,S-,V+,X-}
{$M 4096,0,4096}
UNIT TLDATA;
INTERFACE

TYPE TLString = string[255];
     TLIdent  = string[12];
     BytePtr=^Byte;
     Str2=string[2];
     Str80=string[80];
     SizedPtr = ^SizedRec;
     SizedRec = RECORD
       size:Word;
       buf:array[0..1] OF Byte;
     END;
     TLDataType = (
       dt_STRUC,dt_SSTRUC,         (* unknown structure *)
       dt_STRING,dt_SSTRING,       (* ascii chars       *)
       dt_BYTE,dt_SBYTE,           (* (un)signed Byte   *)
       dt_WORD,dt_SWORD,           (* (un)signed Word   *)
       dt_LONG,dt_SLONG,           (* (un)signed Long   *)
       dt_REAL4,dt_SREAL4,         (* (un)signed Real*4 *)
       dt_REAL8,dt_SREAL8,         (* (un)signed Real*8 *)
       dt_BROKEN,dt_SBROKEN,       (* (un)signed Real*6 *)
       dt_BIT8,dt_SBIT8,           (* (un)signed Bit8   *)
       dt_BIT16,dt_SBIT16,         (* (un)signed Bit16  *)
       dt_TEXTF,dt_STEXTF          (* (un)signed TextF  *)
       );
     BrokenRec = RECORD
       mantissa:LongInt;
       exponent:Integer;
     END;

FUNCTION get_integer(dt:TLDataType; s:Integer; p:Pointer):LongInt;
FUNCTION get_double(dt:TLDataType;s:Integer;p:Pointer):double;
FUNCTION dtyp_size(dt:TLDataType):word;

IMPLEMENTATION

TYPE DFunc=function(p:Pointer; s:Integer):double;
TYPE LFunc=function(p:Pointer; s:Integer):LongInt;


CONST dtyp_sizes:ARRAY [TLDataType] OF Integer=(
  0,0,    (* STRUC  *)
  0,0,    (* String *)
  1,1,    (* Byte   *)
  2,2,    (* Word   *)
  4,4,    (* Long   *)
  4,4,    (* Real4  *)
  8,8,    (* Real8  *)
  6,6,    (* Real6  *)
  1,1,    (* Bit8   *)
  2,2,    (* Bit16  *)
  0,0 );  (* TextF  *)

CONST bmasken:array[0..15] OF Word=(
  $0001,$0003,$0007,$000F,$001F,$003F,$007F,$00FF,
  $01FF,$03FF,$07FF,$0FFF,$1FFF,$3FFF,$7FFF,$FFFF);
CONST bvorz:array[0..15] OF Word=(
 $0001,$0002,$0004,$0008,$0010,$0020,$0040,$0080,
 $0100,$0200,$0400,$0800,$1000,$2000,$4000,$8000);

FUNCTION rest_2_d(p:Pointer; s:Integer):double;
BEGIN
 rest_2_d:=0;
END;
FUNCTION dnil(p:Pointer; s:Integer):double;
BEGIN
 dnil:=0;
END;
FUNCTION lnil(p:Pointer; s:Integer):LongInt;
BEGIN
 lnil:=0;
END;
FUNCTION brkn_2_d(p:Pointer; s:Integer):double;
VAR b:^BrokenRec; dd,d:double; n:Integer;
BEGIN
  b:=p;
  d:=b^.mantissa; n:=b^.exponent;
  d:=d/(2.0*$40000000);
  (* broken_2_d:=ldexp(d,b.exponent); *)
  (* dd:=1 SHL b^.exponent; *)
  dd:=1;
  WHILE (n>0) DO BEGIN dd:=dd*2; Dec(n); END;
  WHILE (n<0) DO BEGIN dd:=dd/2; Inc(n); END;
  brkn_2_d:=d*dd;
END;
FUNCTION dble_2_d(p:Pointer; s:Integer):double;
VAR b:^double;
BEGIN
  b:=p; dble_2_d:=b^;
END;
FUNCTION sngl_2_d(p:Pointer; s:Integer):double;
VAR b:^single;
BEGIN
  b:=p; sngl_2_d:=b^;
END;
FUNCTION slng_2_d(p:Pointer; s:Integer):LongInt;
VAR b:^LongInt;
BEGIN
  b:=p; slng_2_d:=b^;
END;
FUNCTION long_2_d(p:Pointer; s:Integer):double;
VAR b:^LongInt;
BEGIN
  b:=p; long_2_d:=b^;
END;
FUNCTION swrd_2_d(p:Pointer; s:Integer):LongInt;
VAR b:^Integer;
BEGIN
  b:=p; swrd_2_d:=b^;
END;
FUNCTION word_2_d(p:Pointer; s:Integer):LongInt;
VAR b:^Word;
BEGIN
  b:=p; word_2_d:=b^;
END;
FUNCTION byte_2_d(p:Pointer; s:Integer):LongInt;
VAR b:^Byte;
BEGIN
  b:=p; byte_2_d:=b^;
END;
FUNCTION sbyt_2_d(p:Pointer; s:Integer):LongInt;
VAR b:^ShortInt;
BEGIN
  b:=p; sbyt_2_d:=b^;
END;
FUNCTION sbt8_2_d(p:Pointer; s:Integer):LongInt;
VAR b:^Byte; sh,m,bvm:word; x:Integer;
BEGIN
  b:=p; x:=b^;
  sh:=s AND $0F; m:=(s SHR 8) AND $0F;
  bvm:=bvorz[m]; m:=bmasken[m];
  x:=(x SHR sh) AND m;
  IF (x AND bvm)=bvm THEN x:=x OR (NOT m);
  sbt8_2_d:=x;
END;
FUNCTION sbt6_2_d(p:Pointer; s:Integer):LongInt;
VAR b:^Word; sh,m,bvm:word; x:Integer;
BEGIN
  b:=p; x:=b^;
  sh:=s AND $0F; m:=(s SHR 8) AND $0F;
  bvm:=bvorz[m]; m:=bmasken[m];
  x:=(x SHR sh) AND m;
  IF (x AND bvm)=bvm THEN x:=x OR (NOT m);
  sbt6_2_d:=x;
END;
FUNCTION bit8_2_d(p:Pointer; s:Integer):LongInt;
VAR b:^Byte; sh,m:word; x:Word;
BEGIN
  b:=p; x:=b^;
  sh:=s AND $0F; m:=(s SHR 8) AND $0F;
  m:=bmasken[m];
  x:=(x SHR sh) AND m;
  bit8_2_d:=x;
END;
FUNCTION bit6_2_d(p:Pointer; s:Integer):LongInt;
VAR b:^Word; sh,m:word; x:Word;
BEGIN
  b:=p; x:=b^;
  sh:=s AND $0F; m:=(s SHR 8) AND $0F;
  m:=bmasken[m];
  x:=(x SHR sh) AND m;
  bit6_2_d:=x;
END;

CONST dbl_funcs:Array[TLDataType] OF DFunc=(
   rest_2_d, rest_2_d, (* dt_STRUC,dt_SSTRUC,   *)
   rest_2_d, rest_2_d, (* dt_STRING,dt_SSTRING, *)
   dnil    , dnil,     (* dt_BYTE,dt_SBYTE,     *)
   dnil    , dnil,     (* dt_WORD,dt_SWORD,     *)
   long_2_d, dnil,     (* dt_LONG,dt_SLONG,     *)
   sngl_2_d, sngl_2_d, (* dt_REAL4,dt_SREAL4,   *)
   dble_2_d, dble_2_d, (* dt_REAL8,dt_SREAL8,   *)
   brkn_2_d, brkn_2_d, (* dt_BROKEN,dt_SBROKEN, *)
   dnil    , dnil,     (* dt_BIT8,dt_SBIT8,     *)
   dnil    , dnil,     (* dt_BIT16,dt_SBIT16,   *)
   rest_2_d, rest_2_d  (* dt_TEXTF,dt_STEXTF    *) );
CONST int_funcs:Array[TLDataType] OF LFunc=(
   lnil    , lnil    , (* dt_STRUC,dt_SSTRUC,   *)
   lnil    , lnil    , (* dt_STRING,dt_SSTRING, *)
   byte_2_d, sbyt_2_d, (* dt_BYTE,dt_SBYTE,     *)
   word_2_d, swrd_2_d, (* dt_WORD,dt_SWORD,     *)
   lnil    , slng_2_d, (* dt_LONG,dt_SLONG,     *)
   lnil    , lnil    , (* dt_REAL4,dt_SREAL4,   *)
   lnil    , lnil    , (* dt_REAL8,dt_SREAL8,   *)
   lnil    , lnil    , (* dt_BROKEN,dt_SBROKEN, *)
   bit8_2_d, sbt8_2_d, (* dt_BIT8,dt_SBIT8,     *)
   bit6_2_d, sbt6_2_d, (* dt_BIT16,dt_SBIT16,   *)
   lnil    , lnil      (* dt_TEXTF,dt_STEXTF    *) );

FUNCTION get_integer(dt:TLDataType; s:Integer; p:Pointer):LongInt;
BEGIN
  IF @int_funcs[dt]=@lnil THEN get_integer:=Round(dbl_funcs[dt](p,s))
  ELSE                         get_integer:=int_funcs[dt](p,s);
END;
FUNCTION get_double(dt:TLDataType;s:Integer;p:Pointer):double;
BEGIN
  IF @dbl_funcs[dt]=@dnil THEN get_double:=int_funcs[dt](p,s)
  ELSE                         get_double:=dbl_funcs[dt](p,s);
END;
FUNCTION dtyp_size(dt:TLDataType):word;
BEGIN
  dtyp_size:=dtyp_sizes[dt];
END;
BEGIN
END.

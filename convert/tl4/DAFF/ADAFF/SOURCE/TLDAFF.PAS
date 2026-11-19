{$A+,B-,D+,E+,F+,G-,I-,L+,N+,O-,R-,S-,V+,X-}
{$M 4096,0,4096}
UNIT TLDAFF;

INTERFACE
USES TLData;


CONST DAFF_SMALL_SIZE=$FFFF;
      DAFF_LONG_SIZE=$FFFFFFFF;
      DAFF_DONT_KNOW=DAFF_LONG_SIZE;
      DAFF_CONTEXT=$8000;
      DAFF_ECTX=$FFFF;
      DAFF_EOF=$FFFE;

TYPE DaffError=(DAFF_NO_ERR,
                DAFF_MEM_ERR,DAFF_FNF_ERR,DAFF_CNC_ERR,
                DAFF_RD_ERR,DAFF_WR_ERR,DAFF_EOF_ERR,
                DAFF_PAR_ERR,DAFF_NESTING_ERR,
                DAFF_ILL_ERR,DAFF_SEX_ERR);

CONST DaffErrorTexte:array [DaffError] OF String=(
  'Kein Fehler', 'Zu wenig Speicher', 'Datei nicht gefunden',
  'Datei kann nicht erstellt werden', 'Lese-Fehler in Datei',
  'Schreib-Fehler auf Datei', 'Unerwartetes Ende der Datei erreicht',
  'Parameter-Fehler in Datei', 'UngÅltiges Nesting',
  'UngÅltige Daff-Datei','Nicht unterstÅtztes Daff-Format');

TYPE DaffIdent=ARRAY[1..10] OF byte;
     DaffHeaderRec = RECORD
       swg: DaffIdent;
       sex,version,flags:Word;
       size:LongInt;
       res: ARRAY[0..7] OF Byte;
     END;
     TwoLongsRec=RECORD
       size,temp:LongInt;
     END;
     TagRec = RECORD
       tag_id:word;            (* id_byte of this tag/context *)
       tag_num:word;           (* entity_id or specific param *)
       tag_dtyp:TLDataType;    (* see enum above              *)
       tag_dummy:TLDataType;   (* see enum above              *)
       tag_size:word;          (* size in bytes/0xFFFF        *)
       case Integer OF
       0: (longs:TwoLongsRec);
       1: (broken:BrokenRec);
       2: (dval:double);
       3: (bytes:array[0..7] OF Byte);
     END;
    DAF = RECORD
      f:FILE;
      t:TagRec;
      error:DaffError;
      header:DaffHeaderRec;
    END;

FUNCTION daff_read_str_tag(VAR f:DAF):TLString;
FUNCTION daff_read_int_tag(VAR f:DAF):LongInt;
FUNCTION daff_read_dbl_tag(VAR f:DAF):double;
FUNCTION daff_read_void_tag(VAR f:DAF):SizedPtr;
FUNCTION daff_read_tag(VAR f:DAF):Word;
FUNCTION daff_read_ctx(VAR f:DAF):Word;
FUNCTION daff_tag_size(VAR f:DAF):LongInt;
PROCEDURE daff_skip_tag(VAR f:DAF);
PROCEDURE daff_skip_ctx(VAR f:DAF);
FUNCTION daff_open(VAR f:DAF; fn:string):DaffError;
PROCEDURE daff_close(VAR f:DAF);
FUNCTION daff_get_pos(VAR f:DAF):LongInt;
PROCEDURE daff_clr_dbl_tag(VAR f:DAF);

IMPLEMENTATION

CONST DAFF_ID='SWGBSMBWS';
      DAFF_SEX_INTEL= (Ord('L') OR (Ord('H') SHL 8));
      DAFF_SEX_MOTOROLA= (Ord('H') OR (Ord('L') SHL 8));
      DAFF_VERSION=$0100;


PROCEDURE r_ead(VAR f:DAF; VAR buf; n:Word);
  VAR nRead:word;
  BEGIN
   BlockRead(f.f,buf,n,nRead);
   IF nRead=0 THEN f.error:=DAFF_EOF_ERR
   ELSE IF nRead<>n THEN f.error:=DAFF_RD_ERR;
  END;
PROCEDURE w_rite(VAR f:DAF; VAR buf; n:Word);
  VAR nRead:word;
  BEGIN
   BlockWrite(f.f,buf,n,nRead);
   IF nRead<>n THEN f.error:=DAFF_RD_ERR;
  END;

FUNCTION cscmp(VAR p:DaffIdent;s:TLString):Boolean;
VAR n,i:integer; b:Boolean;
BEGIN
  b:=TRUE; i:=1;
  n:=Length(s);
  IF n>10 THEN n:=10;
  WHILE b AND (i<n) DO BEGIN
    b:=p[i]=Ord(s[i]);
    i:=Succ(i);
  END;
  cscmp:=b;
END;

(* ---------------- INPUT ------------------------ *)
FUNCTION daff_read_str_tag(VAR f:DAF):TLString;
VAR s:TLString;
    i,len,slen,xlen:word;
BEGIN
  s:='';
  len:=f.t.tag_size;
  IF len>255 THEN BEGIN
    daff_skip_tag(f);
    f.error:=DAFF_PAR_ERR;
  END
  ELSE IF len<=8 THEN BEGIN
    FOR i:=0 TO len-1 DO
     s:=s+Char(f.t.bytes[i]);
  END
  ELSE BEGIN
    IF Odd(len) THEN xlen:=Succ(len)
    ELSE             xlen:=len;
    WHILE xlen>0 DO BEGIN
      IF xlen>=8 THEN slen:=8
      ELSE            slen:=xlen;
      r_ead(f,f.t.bytes,slen);
      IF f.error=DAFF_RD_ERR THEN xlen:=0
      ELSE BEGIN
        xlen:=xlen-slen;
        IF len<slen THEN slen:=len;
        FOR i:=0 TO slen-1 DO BEGIN
          s:=s+Char(f.t.bytes[i]);
          len:=Pred(len);
        END
      END
    END
  END;
  daff_read_str_tag:=s;
END;

FUNCTION daff_read_int_tag(VAR f:DAF):LongInt;
VAR dt:TLDataType;
BEGIN
  dt:=f.t.tag_dtyp;
  IF (dt<dt_BYTE) OR (dt>dt_SBROKEN) THEN
  BEGIN
    f.error:=DAFF_PAR_ERR;
    daff_read_int_tag:=0;
  END
  ELSE IF f.t.tag_size<>dtyp_size(dt) THEN
  BEGIN
    f.error:=DAFF_PAR_ERR;
    daff_read_int_tag:=0
  END
  ELSE daff_read_int_tag:=get_integer(dt,0,@f.t.bytes);
END;
FUNCTION daff_read_dbl_tag(VAR f:DAF):double;
VAR dt:TLDataType;
BEGIN
  dt:=f.t.tag_dtyp;
  IF (dt<dt_BYTE) OR (dt>dt_SBROKEN) THEN
  BEGIN
    f.error:=DAFF_PAR_ERR;
    daff_read_dbl_tag:=0;
  END
  ELSE IF f.t.tag_size<>dtyp_size(dt) THEN
  BEGIN
    f.error:=DAFF_PAR_ERR;
    daff_read_dbl_tag:=0
  END
  ELSE daff_read_dbl_tag:=get_double(dt,0,@f.t.bytes);
END;
PROCEDURE daff_clr_dbl_tag(VAR f:DAF);
VAR dt:TLDataType;
BEGIN
  dt:=f.t.tag_dtyp;
  IF (dt<dt_BYTE) OR (dt>dt_SBROKEN) THEN
  BEGIN
    f.error:=DAFF_PAR_ERR;
  END
  ELSE IF f.t.tag_size<>dtyp_size(dt) THEN
  BEGIN
    f.error:=DAFF_PAR_ERR;
  END
  ELSE BEGIN
   f.t.dval:=0;
   Seek(f.f,FilePos(f.f)-sizeof(TagRec));
   w_rite(f,f.t,Sizeof(f.t));
  END
END;

FUNCTION daff_read_void_tag(VAR f:DAF):SizedPtr;
VAR len:Word; p:SizedPtr;
BEGIN
  len:=f.t.tag_size; p:=NIL;
  IF len<>0 THEN BEGIN
    IF len=DAFF_SMALL_SIZE THEN f.error:=DAFF_PAR_ERR
    ELSE BEGIN
      IF (len>8) AND Odd(len) THEN len:=Succ(len);
      GetMem(p,len+4);
      IF p=NIL THEN f.error:=DAFF_MEM_ERR
      ELSE BEGIN
        FillChar(p^,0,len+4);
        p^.size:=len+4;
        IF len<=0 THEN Move(p^.buf,f.t.bytes,len)
        ELSE           r_ead(f,p^.buf,len);
      END
    END
  END;
  daff_read_void_tag:=p;
END;


FUNCTION daff_read_tag(VAR f:DAF):Word;
BEGIN
  r_ead(f,f.t,Sizeof(f.t));
  IF f.error<>DAFF_NO_ERR THEN
  BEGIN
    IF f.error=DAFF_EOF_ERR
    THEN daff_read_tag:=DAFF_EOF
    ELSE daff_read_tag:=0
  END
  ELSE daff_read_tag:=f.t.tag_id;
END;

PROCEDURE daff_skip_tag(VAR f:DAF);
VAR size:LongInt;
BEGIN
  size:=f.t.tag_size;
  IF size=DAFF_SMALL_SIZE THEN size:=f.t.longs.size;
  IF size=DAFF_LONG_SIZE THEN Seek(f.f,FileSize(f.f))
  ELSE IF size>8 THEN BEGIN
   IF Odd(size) THEN Inc(size);
   Seek(f.f,FilePos(f.f)+size);
  END
END;

PROCEDURE daff_skip_ctx(VAR f:DAF);
VAR err:Integer; id:word;
BEGIN
  IF f.t.tag_size<>0 THEN daff_skip_tag(f)
  ELSE BEGIN
    err:=-1;
    REPEAT
      id:=daff_read_tag(f);
      IF id=0 THEN err:=1
      ELSE IF id=DAFF_ECTX THEN err:=0
      ELSE BEGIN
        IF (id AND DAFF_CONTEXT)<>0
        THEN daff_skip_ctx(f)
        ELSE daff_skip_tag(f);
        IF f.error<>DAFF_NO_ERR THEN err:=1;
      END
    UNTIL err>=0;
  END
END;


FUNCTION daff_open(VAR f:DAF; fn:string):DaffError;
BEGIN
  FillChar(f,SizeOf(f),0);
  Assign(f.f,fn); Reset(f.f,1);
  IF IOResult<>0 THEN daff_open:=DAFF_FNF_ERR (* File not found *)
  ELSE BEGIN
    r_ead(f,f.header,SizeOf(DaffHeaderRec));
    IF (f.error<>DAFF_NO_ERR) OR
       (NOT cscmp(f.header.swg,DAFF_ID)) THEN
    BEGIN
      daff_open:=DAFF_ILL_ERR;
      Close(f.f);
    END
    ELSE IF f.header.sex<>DAFF_SEX_INTEL THEN
    BEGIN
      daff_open:=DAFF_SEX_ERR;
      Close(f.f);
    END
    ELSE daff_open:=DAFF_NO_ERR;
  END
END;

FUNCTION daff_read_ctx(VAR f:DAF):Word;
VAR id:Word;
BEGIN
  id:=daff_read_tag(f);
  IF id<>DAFF_EOF THEN BEGIN
   IF ((id AND DAFF_CONTEXT)=0) OR (id=DAFF_ECTX) THEN id:=0
   ELSE id:=id AND (NOT DAFF_CONTEXT)
  END;
  daff_read_ctx:=id;
END;

FUNCTION daff_tag_size(VAR f:DAF):LongInt;
BEGIN
  IF f.t.tag_size=DAFF_SMALL_SIZE
  THEN daff_tag_size:=f.t.longs.size
  ELSE daff_tag_size:=f.t.tag_size;
END;

PROCEDURE daff_close(VAR f:DAF);
 BEGIN
  Close(f.f);
 END;
FUNCTION daff_get_pos(VAR f:DAF):LongInt;
BEGIN
  daff_get_pos:=FilePos(f.f);
END;

BEGIN
END.

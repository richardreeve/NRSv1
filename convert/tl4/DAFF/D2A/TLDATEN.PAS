UNIT TLDaten;

INTERFACE
USES TLData,TLDaff;

TYPE RangeRec = RECORD
       min,max,         (* Minimum/Maximum *)
       fak,off:double;  (* v*faktor+offset *)
       einh:string[50]; (* Benennung       *)
     END;

     TLSignalPtr = ^TLSignal;
     TLDateiPtr  = ^TLDatei;

     TLSignal = RECORD
       pNext:TLSignalPtr;     (* fÅr Queue-Verwaltung     *)
       kenn,nr:word;          (*      - " -               *)
       name:TLIdent;          (* Name des Signals         *)
       lable:TLString;        (* Und Bemerkung            *)
       dtyp:TLDataType;       (* DatenTyp                 *)
       wsize,size:word;       (* Grî·e des -"-            *)
       blkoffs:word;          (* Offset im Block/Scan     *)
       demux:word;            (* Versatz im Block/Scan    *)
       wpblk:word;            (* Werte pro Block          *)
       anzahl,block0:LongInt; (* Anzahl Werte vorhanden   *)
       x,y,ad,usr:RangeRec;   (* Umrechnungen             *)
       pDatei:TLDateiPtr;     (* Aus welcher Datei        *)
       sweep:double;          (* Sweep-LÑnge              *)
     END;
     TLDatei = RECORD
      pNext:TLDateiPtr;       (* fÅr Queue-Verwaltung         *)
      kenn,nr:word;           (*  - " -                       *)
      name:TLIdent;           (* TL-Name der Datei            *)
      lable:TLString;         (* Bemerkung                    *)
      typ:word;               (* Flags fÅr DateiTyp           *)
      inuse,open:word;        (* ZÑhler: gebraucht,geîffnet   *)
      flags:word;             (* mem/dup/blk                  *)
      Path:TLString;          (* Voller Pfadname              *)
      DDFPath:TLString;       (* Pfadname (*.DDF)             *)
      datum:LongInt;          (* date/time (Unix-Format)      *)
      blksize:word;           (* BlockSize in Byte/Werte      *)
      blksizb:word;           (* BlockSize in Byte            *)
      hdrsize:LongInt;        (* Header-Grî·e in Byte         *)
      hdrlines:word;          (* Header-Grî·e in Zeilen       *)
      datasize:LongInt;       (* Daten-Grî·e in Bytes         *)
      nblocks:LongInt;        (*  - " -      in Blîcken       *)
      nsignale:word;          (* Anzahl Signale in Datei      *)
      pSigs:TLSignalPtr;      (* Queue der Signale            *)
      ddaf:DAF;               (* DAF-Datei                    *)
      buffer:Pointer;         (* Allozierter Buffer           *)
      RecNo:LongInt;          (* Aktueller Record             *)
      xSet:Boolean;           (* xMin<>xMax<>0 !              *)
      END;

FUNCTION  OpenDaffDatei(VAR d:TLDatei; fn:TLString):DaffError;
PROCEDURE CloseDaffDatei(VAR d:TLDatei);
FUNCTION  ReadNextRecord(VAR d:TLDatei):Boolean;
FUNCTION  QueryRecordTime(VAR d:TLDatei; f:integer):double;
FUNCTION  QuerySignalValue(VAR d:TLDatei; sn,f:integer):double;
FUNCTION  SeekRecord(VAR d:TLDatei; rnr:LongInt):LongInt;
PROCEDURE XResetDaffDatei(VAR d:TLDatei);

IMPLEMENTATION

CONST erg_ANZAHL=256;
      erg_BLOCK0=257;
CONST XCHECK=1;
      YCHECK=2;

CONST tag_ERR=0;
      tag_NAME=1;    tag_LABEL=2;  tag_DATE=3; tag_VALUE=4; (* 1-4     *)
      tag_FTYP=128;  tag_PROD=129; tag_FACT=130;            (* 128-130 *)
      tag_OFFS=131;  tag_SPEC=132; tag_MINV=133;            (* 131-133 *)
      tag_MAXV=134;  tag_MEAN=135; tag_SDEV=136;            (* 134-136 *)
      tag_SWEEP=137; tag_SFRQ=138;                          (* 137-138 *)
      tag_SOURCE=150;tag_SRCOFFS=151;tag_BLKSIZE=152;       (* 150-152 *)
      tag_DSIZE=153; tag_SRCTYP=154;                        (* 153-154 *)
      tag_BLKOFFS=160;tag_DEMUX=161;tag_DATAT=162;          (* 160-162 *)
      tag_DTYPS=163; tag_DARGS =164;tag_WPBLK=165;          (* 163-165 *)
CONST ctx_ERR=0;
      ctx_CTXDEF=1; ctx_HEADER=2; ctx_DATADEF=3; ctx_DATA=4;
      ctx_ENTITY=5; ctx_DOCUM =6; ctx_RESULTS=7;            (* 1- 7    *)

FUNCTION SignalByNr(s:TLSignalPtr; nr:word):TLSignalPtr;
VAR found:Boolean;
BEGIN
  found:=FALSE;
  WHILE (NOT found) AND (s<>NIL) DO
  BEGIN
    found:=s^.nr=nr;
    IF NOT found THEN s:=s^.pNext;
  END;
  SignalByNr:=s;
END;
PROCEDURE AppendSig(VAR b,s:TLSignalPtr);
VAR p:TLSignalPtr;
BEGIN
  IF b=NIL THEN b:=s
  ELSE BEGIN
    p:=b;
    WHILE p^.pNext<>NIL DO p:=p^.pNext;
    p^.pNext:=s;
  END;
  s^.pNext:=NIL;
END;

(* ------ read HEADER-Context of DAFF-File hf --------- *)
(*  tag_DATE : d.datum:=daff_read_date_tag(hf);         *)
PROCEDURE read_header(VAR hf:DAF; VAR d:TLDatei);
VAR tid:Word;
BEGIN
  WHILE hf.error=DAFF_NO_ERR DO
  BEGIN
    tid:=daff_read_tag(hf);
    IF tid=DAFF_ECTX THEN Exit
    ELSE IF (tid AND DAFF_CONTEXT)<>0 THEN daff_skip_ctx(hf)
    ELSE CASE tid OF
    tag_LABEL: d.lable:=daff_read_str_tag(hf);
    tag_NAME : d.name :=daff_read_str_tag(hf);
    ELSE       daff_skip_tag(hf);
    END
  END
END;

(* ------ read Channel Definition from DAFF-File hf ----- *)
PROCEDURE read_channel(VAR hf:DAF; s:TLSignalPtr);
VAR tid:Word; v:double;
BEGIN
  WHILE hf.error=DAFF_NO_ERR DO
  BEGIN
    tid:=daff_read_tag(hf);
    IF tid=DAFF_ECTX THEN Exit
    ELSE IF (tid AND DAFF_CONTEXT)<>0 THEN  daff_skip_ctx(hf)
    ELSE CASE tid OF
    tag_NAME:  s^.name:=daff_read_str_tag(hf);
    tag_LABEL: s^.lable:=daff_read_str_tag(hf);
    tag_SPEC:  CASE hf.t.tag_num OF
               0: s^.x.einh:=daff_read_str_tag(hf);
               1: s^.ad.einh:=daff_read_str_tag(hf);
               2: s^.usr.einh:=daff_read_str_tag(hf);
               else daff_skip_tag(hf);
               END;
    tag_FACT:  CASE hf.t.tag_num OF
               0: s^.x.fak:=daff_read_dbl_tag(hf);
               1: s^.ad.fak:=daff_read_dbl_tag(hf);
               2: s^.usr.fak:=daff_read_dbl_tag(hf);
               else daff_skip_tag(hf);
               END;
    tag_OFFS:  CASE hf.t.tag_num OF
               0: s^.x.off:=daff_read_dbl_tag(hf);
               1: s^.ad.off:=daff_read_dbl_tag(hf);
               2: s^.usr.off:=daff_read_dbl_tag(hf);
               else daff_skip_tag(hf);
               END;
    tag_MINV:  CASE hf.t.tag_num OF
               0: s^.x.min:=daff_read_dbl_tag(hf);
               1: s^.ad.min:=daff_read_dbl_tag(hf);
               2: s^.usr.min:=daff_read_dbl_tag(hf);
               else daff_skip_tag(hf);
               END;
    tag_MAXV:  CASE hf.t.tag_num OF
               0: s^.x.max:=daff_read_dbl_tag(hf);
               1: s^.ad.max:=daff_read_dbl_tag(hf);
               2: s^.usr.max:=daff_read_dbl_tag(hf);
               else daff_skip_tag(hf);
               END;
    tag_SFRQ:  BEGIN
                 v:=daff_read_dbl_tag(hf);
                 IF v<>0 THEN
                 BEGIN
                  s^.x.fak:=1.0/v; s^.x.off:=0;
                  s^.x.min:=0; s^.x.max:=0;
                  s^.x.einh:='sec';
                 END
               END;
    tag_SWEEP: s^.sweep:=daff_read_dbl_tag(hf);
    else        daff_skip_tag(hf);
    END
  END
END;

(* ------ reset xmin/xmax of cahnnels ----- *)
PROCEDURE reset_channel(VAR hf:DAF);
VAR tid:Word; v:double;
BEGIN
  WHILE hf.error=DAFF_NO_ERR DO
  BEGIN
    tid:=daff_read_tag(hf);
    IF tid=DAFF_ECTX THEN Exit
    ELSE IF (tid AND DAFF_CONTEXT)<>0 THEN  daff_skip_ctx(hf)
    ELSE BEGIN
      CASE tid OF
      tag_MINV:  IF hf.t.tag_num=0 THEN daff_clr_dbl_tag(hf);
      tag_MAXV:  IF hf.t.tag_num=0 THEN daff_clr_dbl_tag(hf);
      END;
      daff_skip_tag(hf);
    END
  END
END;

(* Ein Entity-Block besteht aus lauter Entity-Blîcken mit num=id *)
PROCEDURE read_entity(VAR hf:DAF; VAR d:TLDatei);
VAR sd:TLSignal; s:TLSignalPtr;
    id,tid:Word;
BEGIN
  FillChar(sd,SizeOf(TLSignal),0);
  WHILE hf.error=DAFF_NO_ERR DO
  BEGIN
    tid:=daff_read_tag(hf);
    IF tid=DAFF_ECTX THEN Exit
    ELSE IF (tid AND DAFF_CONTEXT)=0 THEN daff_skip_tag(hf)
    ELSE IF (tid AND (NOT DAFF_CONTEXT))<>ctx_ENTITY THEN
    BEGIN
      daff_skip_ctx(hf);
    END
    ELSE BEGIN
      IF hf.t.tag_num=0 THEN s:=@sd
      ELSE BEGIN
        s:=SignalByNr(d.pSigs,hf.t.tag_num);
        IF s=NIL THEN BEGIN
          GetMem(s,Sizeof(TLSignal));
          IF s=NIL THEN hf.error:=DAFF_MEM_ERR
          ELSE BEGIN
            s^:=sd;
            s^.nr:=hf.t.tag_num;
            s^.pdatei:=@d;
            s^.pNext:=NIL;
            AppendSig(d.pSigs,s);
          END
        END
      END;
      IF s<>NIL THEN read_channel(hf,s)
      ELSE daff_skip_ctx(hf);
    END
  END
END;

(* Ein Entity-Block besteht aus lauter Entity-Blîcken mit num=id *)
PROCEDURE reset_entity(VAR hf:DAF);
VAR  id,tid:Word;
BEGIN
  WHILE hf.error=DAFF_NO_ERR DO
  BEGIN
    tid:=daff_read_tag(hf);
    IF tid=DAFF_ECTX THEN Exit
    ELSE IF (tid AND DAFF_CONTEXT)=0 THEN daff_skip_tag(hf)
    ELSE IF (tid AND (NOT DAFF_CONTEXT))<>ctx_ENTITY THEN
    BEGIN
      daff_skip_ctx(hf);
    END
    ELSE BEGIN
      reset_channel(hf)
    END
  END
END;

PROCEDURE read_sigdef(VAR hf:DAF; s:TLSignalPtr);
VAR tid:Word;
BEGIN
  WHILE hf.error=DAFF_NO_ERR DO
  BEGIN
    tid:=daff_read_tag(hf);
    IF tid=DAFF_ECTX THEN Exit
    ELSE IF (tid AND DAFF_CONTEXT)<>0 THEN  daff_skip_ctx(hf)
    ELSE CASE tid OF
    tag_BLKOFFS: s^.blkoffs:=daff_read_int_tag(hf);
    tag_DEMUX:   s^.demux:=daff_read_int_tag(hf);
    tag_DATAT:   s^.dtyp:=TLDataType(daff_read_int_tag(hf));
    tag_DTYPS:   s^.wsize:=daff_read_int_tag(hf);
    tag_DARGS:   s^.size:=daff_read_int_tag(hf);
    tag_WPBLK:   s^.wpblk:=daff_read_int_tag(hf);
    erg_ANZAHL:  s^.anzahl:=daff_read_int_tag(hf);
    erg_BLOCK0:  s^.block0:=daff_read_int_tag(hf);
    else        daff_skip_tag(hf);
    END
  END
END;

PROCEDURE read_datadef(VAR hf:DAF; VAR d:TLDatei);
VAR tid,id:Word; s:TLSignalPtr;
BEGIN
  WHILE hf.error=DAFF_NO_ERR DO
  BEGIN
    tid:=daff_read_tag(hf);
    IF tid=DAFF_ECTX THEN exit
    ELSE IF (tid AND DAFF_CONTEXT)<>0 THEN
    BEGIN
      IF (tid AND (NOT DAFF_CONTEXT))=ctx_DATADEF THEN
      BEGIN
        s:=SignalByNr(d.pSigs,hf.t.tag_num);
        IF s=NIL THEN hf.error:=DAFF_PAR_ERR
        ELSE read_sigdef(hf,s);
      END
      ELSE daff_skip_ctx(hf);
    END
    ELSE CASE tid OF
    tag_SOURCE: d.path:=daff_read_str_tag(hf);
    tag_SRCOFFS: d.hdrsize:=daff_read_int_tag(hf);
    tag_BLKSIZE: d.blksize:=daff_read_int_tag(hf);
    tag_SRCTYP : d.typ:=daff_read_int_tag(hf);
    else         daff_skip_tag(hf);
    END
  END
END;

PROCEDURE read_datei(VAR hf:DAF; VAR d:TLDatei);
VAR needed:Integer; blocked:Boolean;
    s:TLSignalPtr; ctx:Word;
BEGIN
  needed:=0; blocked:=FALSE;
  WHILE (needed<>3) AND (hf.error=DAFF_NO_ERR) DO
  BEGIN
    ctx:=daff_read_ctx(hf);
    IF (ctx=0) OR (ctx=DAFF_EOF) THEN hf.error:=DAFF_ILL_ERR
    ELSE CASE ctx OF
    ctx_HEADER:  read_header(hf,d);
    ctx_ENTITY:  read_entity(hf,d);
    ctx_DATADEF: BEGIN
                   read_datadef(hf,d);
                   needed:=needed OR 1;
                   IF d.path<>'' THEN needed:=needed OR 2;
                   (* QLB-Dateien werden NICHT unterstÅtzt *)
                   IF d.path<>'' THEN hf.error:=DAFF_ILL_ERR;
                 END;
    ctx_DATA:    BEGIN
                   IF needed=0 THEN hf.error:=DAFF_ILL_ERR
                   ELSE BEGIN
                     d.hdrsize:=daff_get_pos(hf);
                     d.datasize:=daff_tag_size(hf);
                     IF (d.datasize=DAFF_DONT_KNOW) OR
                        (d.datasize=0) THEN
                     BEGIN
                       d.datasize:=FileSize(hf.f)-d.hdrsize;
                     END
                     ELSE d.datasize:=d.datasize-SizeOf(TagRec);
                     needed:=3;
                   END
                 END;
    ELSE daff_skip_ctx(hf);
    END
  END;
  IF hf.error=DAFF_NO_ERR THEN
  BEGIN
   s:=d.pSigs;
   WHILE s<>NIL DO
   BEGIN
     IF s^.wpblk<=1 THEN s^.wpblk:=1
     ELSE blocked:=TRUE;
     IF s^.dtyp<dt_TEXTF THEN s^.wsize:=dtyp_size(s^.dtyp)
     ELSE                     s^.wsize:=s^.size;
     s:=s^.pNext;
   END;
   IF d.typ<>0 THEN  (* ASCII *)
   BEGIN
     d.blksizb:=d.blksize*Sizeof(double);
     d.hdrlines:=d.hdrsize;
     d.hdrsize:=0;
   END
   ELSE BEGIN
     d.blksizb:=d.blksize;
     d.hdrlines:=0;
   END;
   (* ASCII-Dateien und geblockte Dateien werden NICHT unterstÅtzt *)
   IF (d.typ<>0) OR blocked THEN hf.error:=DAFF_ILL_ERR;
  END
END;

PROCEDURE reset_datei(VAR hf:DAF);
VAR needed:Integer;
    ctx:Word; sPos:LongInt;
BEGIN
  needed:=0;
  sPos:=daff_get_pos(hf);
  Seek(hf.f,sizeof(DaffHeaderRec));
  WHILE (needed<>3) AND (hf.error=DAFF_NO_ERR) DO
  BEGIN
    ctx:=daff_read_ctx(hf);
    IF (ctx=0) OR (ctx=DAFF_EOF) THEN hf.error:=DAFF_ILL_ERR
    ELSE CASE ctx OF
    ctx_ENTITY:  reset_entity(hf);
    ctx_DATA:    needed:=3;
    ELSE daff_skip_ctx(hf);
    END
  END;
  Seek(hf.f,sPos);
END;

PROCEDURE XResetDaffDatei(VAR d:TLDatei);
BEGIN
  reset_datei(d.ddaf)
END;

FUNCTION calc_range(x:double; r:RangeRec):double;
BEGIN
  calc_range:=x*r.fak+r.off;
END;
FUNCTION from_range(x:double; r:RangeRec):double;
BEGIN
 IF r.fak=0 THEN from_range:=0
 ELSE from_range:=(x-r.off)/r.fak;
END;
PROCEDURE squeeze_range(usr,ad:RangeRec; VAR r:RangeRec);
BEGIN
  r:=usr; r.off:=r.off+r.fak*ad.off; r.fak:=r.fak*ad.fak;
  IF (usr.fak=1) AND (usr.off=0) AND (usr.einh='') THEN
    r.einh:=ad.einh;
END;

PROCEDURE x_minmax_check(VAR d:TLDatei; blocks:LongInt);
VAR xmin,xmax:double; n:Integer; s:TLSignalPtr;
BEGIN
  n:=0; s:=d.pSigs;
  WHILE s<>NIL DO
  BEGIN
    xmin:=calc_range(0,s^.x);
    IF s^.anzahl<>0
    THEN xmax:=s^.anzahl-1
    ELSE xmax:=blocks*s^.wpblk-1;
    xmax:=calc_range(xmax,s^.x);
    IF (s^.kenn AND XCHECK)<>0 THEN
    BEGIN
      s^.x.min:=xmin; s^.x.max:=xmax;
      s^.kenn:=s^.kenn AND (NOT XCHECK);
    END
    ELSE BEGIN
      IF (s^.x.min<xmin) OR (s^.x.min>=xmax) THEN
      BEGIN
        s^.x.min:=xmin;
        Inc(n);
      END;
      IF (s^.x.max>xmax) OR (s^.x.max<=xmin) THEN
      BEGIN
        IF (s^.x.max-xmax)>s^.x.fak THEN Inc(n);
        s^.x.max:=xmax;
      END
    END;
    s:=s^.pNext;
  END;
  d.nblocks:=blocks;
  (* IF n>0 THEN warnung(INVALID_XRANGE); *)
END;

(*************************************************************
 * Binary-Datei ÅberprÅfen                                   *
 * ----------------------------------------------------------*
 * (a) xprÅfen kann die DateiLÑnge und Blockgrî·e benutzen   *
 * (b) yprÅfen baut eine IOLISTE mit den benîtigten Signalen *
 *     auf, (opcode=YRANGE) und ruft dann  die Prozedur      *
 *     "lesen_und_verarbeiten"                               *
 *************************************************************)

FUNCTION binary_xpruefen(VAR d:TLDatei):LongInt;
VAR blocks,b:LongInt; s:TLSignalPtr;
BEGIN
  blocks:=0; s:=d.pSigs;
  WHILE s<>NIL DO
  BEGIN
    IF s^.anzahl<>0 THEN
    BEGIN
      b:=s^.block0+s^.anzahl*s^.demux;
      IF b>blocks THEN blocks:=b;
    END;
    s:=s^.pNext;
  END;
  IF blocks<>0 THEN
  BEGIN
    d.nblocks:=blocks DIV d.blksizb;
    blocks:=d.nblocks;
  END
  ELSE BEGIN
    d.nblocks:=d.datasize DIV d.blksizb;
    blocks:=d.nblocks;
    (*
    IF (d.datasize MOD d.blksizb)<>0 THEN
      warnung(PARTIAL_BLOCK);
    *)
  END;
  (*
  IF blocks=0 THEN warnung(EMPTY_FILE);
  *)
  binary_xpruefen:=blocks;
END;


FUNCTION binary_datei_pruefen(VAR d:TLDatei; flags:word):LongInt;
VAR blocks:LongInt;
BEGIN
  blocks:=binary_xpruefen(d);
  IF blocks<>0 THEN
  BEGIN
    x_minmax_check(d,blocks);
    (* ----
    IF (flags AND YCHECK)<>0 THEN
    BEGIN
      IF binary_ypruefen(d) THEN blocks:=0;
    END;
    y_minmax_check(d);
    ---- *)
  END;
  binary_datei_pruefen:=blocks;
END;

(*
 * datei_pruefen
 * -------------
 * (1) nicht gÅltige Signal-Namen durch "Signalxx" ersetzen
 * (2) Range-Faktoren (x,usr,ad) dÅrfen nicht 0 sein !
 * (3) Wenn xmin==xmax==0 -> XCHECK nîtig (s->anzahl Null setzen)
 * (4) Wenn ymin==ymax==0 -> YCHECK nîtig
 * (5) Angaben zur Dateistruktur auf Sinnhaftigkeit prÅfen
 * (6) Xrange und (evtl.) YRange ermitteln und einsetzen/prÅfen
 *)
FUNCTION datei_pruefen(VAR d:TLDatei):Boolean;
VAR  flag,sws:word; n,errors:Integer;
     blocks:LongInt; s:TLSignalPtr;
     defnam:TLIdent;
BEGIN
  flag:=0; errors:=0;
  s:=d.pSigs; n:=1;
  d.xSet:=FALSE;
  WHILE s<>NIL DO
  BEGIN
    (*------
    IF invalid_name(s^.name) THEN
    --------*)
    IF s^.name='' THEN
    BEGIN
      Str(n,defnam);
      s^.name:='Signal'+defnam;
    END;
    IF s^.x.fak=0 THEN s^.x.fak:=1.0;
    IF s^.usr.fak=0 THEN s^.usr.fak:=1.0;
    IF s^.ad.fak=0 THEN s^.ad.fak:=1.0;
    squeeze_range(s^.usr,s^.ad,s^.y);
    IF (s^.x.min=0) AND (s^.x.max=0) THEN
    BEGIN
      s^.kenn:=s^.kenn OR XCHECK;
      s^.anzahl:=0;
      flag:=flag OR XCHECK;
    END
    ELSE d.xSet:=TRUE;
    IF (s^.y.min=0) AND (s^.y.max=0) THEN
    BEGIN
      s^.kenn:=s^.kenn OR YCHECK;
      flag:=flag OR YCHECK;
    END;
    (* ----
    IF (d.typ AND dat_ASCII)<>0 THEN sws:=1 ELSE
    ---- *)
    sws:=s^.wsize;
    IF (s^.wpblk=0) OR (s^.wsize=0) OR
       (s^.blkoffs+sws>d.blksize) OR
       (s^.wpblk*s^.demux>d.blksizb) THEN
    BEGIN
      Inc(errors);
    END;
    s:=s^.pNext; Inc(n);
  END;
  d.nsignale:=n-1;
  IF errors=0 THEN
  BEGIN
    (* -----
    IF (d.typ AND dat_ASCII)<>0 THEN blocks:=ascii_datei_pruefen(d,flag)
    ELSE
    ---- *)
         blocks:=binary_datei_pruefen(d,flag);
    IF blocks=0 THEN Inc(errors);
    s:=d.pSigs;
    WHILE s<>NIL DO
    BEGIN
      IF s^.anzahl=0 THEN s^.anzahl:=blocks*s^.wpblk;
      s:=s^.pNext;
    END
  END;
  datei_pruefen:=errors>0;
END;

FUNCTION OpenDaffDatei(VAR d:TLDatei; fn:TLString):DaffError;
VAR e:DaffError;
BEGIN
  FillChar(d,Sizeof(TLDatei),0);
  e:=daff_open(d.ddaf,fn);
  IF e=DAFF_NO_ERR THEN
  BEGIN
   read_datei(d.ddaf,d);
   d.path:=fn;
   IF d.ddaf.error=DAFF_NO_ERR THEN
   BEGIN
     IF datei_pruefen(d) THEN
       d.ddaf.error:=DAFF_PAR_ERR;
   END;
   IF d.ddaf.error=DAFF_NO_ERR THEN
   BEGIN
     GetMem(d.buffer,d.blksizb);
     IF d.buffer=NIL THEN
       d.ddaf.error:=DAFF_MEM_ERR
     ELSE IF SeekRecord(d,0)<>0 THEN ;
   END;
   e:=d.ddaf.error;
   IF e<>DAFF_NO_ERR THEN CloseDaffDatei(d);
  END;
  OpenDaffDatei:=e;
END;

PROCEDURE CloseDaffDatei(VAR d:TLDatei);
VAR ss,s:TLSignalPtr;
BEGIN
  IF d.buffer<>NIL THEN FreeMem(d.buffer,d.blksizb);
  s:=d.pSigs;
  WHILE s<>NIL DO
  BEGIN
    ss:=s;
    s:=s^.pNext;
    FreeMem(ss,Sizeof(TLSignal));
  END;
  daff_close(d.ddaf);
END;

FUNCTION ReadNextRecord(VAR d:TLDatei):Boolean;
BEGIN
  INC(d.RecNo);
  IF d.RecNo > d.nblocks THEN ReadNextRecord:=FALSE
  ELSE BEGIN
    BlockRead(d.ddaf.f,d.buffer^,d.blksizb);
    ReadNextRecord:=TRUE;
  END
END;
FUNCTION SeekRecord(VAR d:TLDatei; rnr:LongInt):LongInt;
BEGIN
  IF (rnr<0) OR (rnr>d.nblocks) THEN rnr:=d.nblocks;
  Seek(d.ddaf.f,rnr*d.blksizb+d.hdrsize);
  SeekRecord:=rnr;
END;

FUNCTION QueryRecordTime(VAR d:TLDatei; f:Integer):double;
VAR x:double;
BEGIN
  IF d.RecNo<=1 THEN QueryRecordTime:=0
  ELSE BEGIN
    x:=d.RecNo-1;
    IF (f > 0) AND (d.pSigs<>NIL) THEN
    BEGIN
      x:=from_range(x,d.psigs^.x);
      IF f>1 THEN x:=x*1000;
    END;
    QueryRecordTime:=x;
  END
END;

FUNCTION QuerySignalValue(VAR d:TLDatei; sn,f:integer):double;
VAR x:double; s:TLSignalPtr; p:Pointer;
BEGIN
  IF (d.RecNo<1) OR (d.RecNo>d.nblocks)  OR
     (sn<1) OR (sn>d.nsignale) THEN QuerySignalValue:=0
  ELSE BEGIN
    s:=d.pSigs;
    WHILE sn>1 DO
    BEGIN
      s:=s^.pNext;
      Dec(sn);
    END;
    p:=Ptr(Seg(d.buffer^),Ofs(d.buffer^)+s^.blkoffs);
    x:=get_double(s^.dtyp,s^.size,p);
    CASE f OF
    1: x:=calc_range(x,s^.ad);
    2: x:=calc_range(x,s^.y);
    END;
    QuerySignalValue:=x;
  END
END;

BEGIN
END.

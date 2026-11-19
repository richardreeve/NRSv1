PROGRAM ADAFF; (* Append to DAFF files *)

(*----------------------------------------------------------------+
 | Aufruf: ADAFF [-o] Datei1[.dat] Datei2[.dat] [Ergebnis[.dat]]  |
 +----------------------------------------------------------------+
 | Semantik:                                                      |
 |   liest die (binÑren) DAFF-Dateien "Datei1" und "Datei2" und   |
 |   vergleicht ihre Header; wenn beide kompatibel sind, dann     |
 |   werden die Werte von "Datei2" an die Datei "Datei1" ange-    |
 |   hÑngt. Das Ergebnis steht in "Ergebnis" bzw. in "Datei1"     |
 | Voreinstellungen:                                              |
 |     alle Dateien haben die Extension ".dat"                    |
 | Optionen:                                                      |
 |   -s+      vergleicht auch Signalnamen                         |
 |   -s- [*]  vergleicht nur die Struktur der Dateien (VE)        |
 |   -d+      lîscht die Dateien "Datei2" und (evtl.) "Datei1"    |
 |   -d- [*]  lîscht die Dateien  nicht (VE)                      |
 |   -v+      gibt zusÑtzliche Meldungen aus                      |
 |   -v- [*]  gibt keine zusÑtzlichen Meldungen aus (VE)          |
 +----------------------------------------------------------------*)

USES Dos,TLData,TLDaff,TLDaten;

CONST DST_CREATE=0;
      DST_APPEND=-1;

PROCEDURE usage;
BEGIN
writeln('*----------------------------------------------------------------+');
writeln('| Aufruf: ADAFF [-o] Datei1[.dat] Datei2[.dat] [Ergebnis[.dat]]  |');
writeln('+----------------------------------------------------------------+');
writeln('| Semantik:                                                      |');
writeln('|   liest die (binÑren) DAFF-Dateien "Datei1" und "Datei2" und   |');
writeln('|   vergleicht ihre Header; wenn beide kompatibel sind, dann     |');
writeln('|   werden die Werte von "Datei2" an die Datei "Datei1" ange-    |');
writeln('|   hÑngt. Das Ergebnis steht in "Ergebnis" bzw. in "Datei1"     |');
writeln('| Voreinstellungen:                                              |');
writeln('|     alle Dateien haben die Extension ".dat"                    |');
writeln('| Optionen:                                                      |');
writeln('|   -s+      vergleicht auch Signalnamen                         |');
writeln('|   -s- [*]  vergleicht nur die Struktur der Dateien (VE)        |');
writeln('|   -d+      lîscht die Dateien "Datei2" und (evtl.) "Datei1"    |');
writeln('|   -d- [*]  lîscht die Dateien  nicht (VE)                      |');
writeln('|   -v+      gibt zusÑtzliche Meldungen aus                      |');
writeln('|   -v- [*]  gibt keine zusÑtzlichen Meldungen aus (VE)          |');
writeln('+----------------------------------------------------------------*');
Halt(1);
END;

PROCEDURE DefaultExtension(VAR fn:PathStr; ext:ExtStr);
VAR
  d: DirStr;
  n: NameStr;
  e: ExtStr;
  i: integer;
BEGIN
  FSplit(fn, d, n, e);
  IF E = '' THEN fn:=fn+ext;
  for i:=1 TO Length(fn) DO
   fn[i]:=UpCase(fn[i]);
end;

FUNCTION NewExtension(fn:PathStr; ext:ExtStr):PathStr;
VAR
  d: DirStr;
  n: NameStr;
  e: ExtStr;
BEGIN
  FSplit(fn, d, n, e);
  NewExtension:=d+n+ext;
END;

FUNCTION CopyFile(VAR sname:PathStr; soff:LongInt;
                  VAR dname:PathStr; doff:LongInt; s_del:Boolean):Boolean;
CONST BUFSIZE = 2048;
VAR
  sf,df:FILE; Error:Boolean;
  size,read:Word; buf:array[1..BUFSIZE] OF Char;

BEGIN
  Error:=FALSE;
  Assign(sf,sname); Assign(df,dname);
  Reset(sf,1);
  IF IOResult<>0 THEN BEGIN
   writeln(^G'Datei ',sname,' kann nicht geîffnet werden');
   Error:=TRUE;
  END
  ELSE BEGIN
    IF doff=DST_CREATE THEN Rewrite(df,1)
    ELSE                    Reset  (df,1);
    IF IOResult<>0 THEN BEGIN
      Close(sf);
      writeln(^G'Datei ',dname,' kann nicht geîffnet werden');
      Error:=TRUE;
    END
    ELSE IF doff=DST_APPEND THEN
    BEGIN
      doff:=FileSize(df);
    END
  END;
  IF NOT Error THEN
  BEGIN
    Seek(sf,soff); Seek(df,doff);
    soff:=FileSize(sf)-soff;
    WHILE (NOT Error) AND (soff>0) DO BEGIN
      IF soff>BUFSIZE THEN size:=BUFSIZE
      ELSE                 size:=soff;
      soff:=soff-size;
      BlockRead(sf,buf,size,read);
      IF size<>read THEN BEGIN
        writeln(^G'LeseFehler in Datei ',sname);
        Error:=TRUE;
      END
      ELSE BEGIN
        BlockWrite(df,buf,size,read);
        IF size<>read THEN BEGIN
          writeln(^G'SchreibFehler in Datei ',dname);
          Error:=TRUE;
        END
      END
    END;
    Close(sf);
    Close(df);
    IF Error AND (doff=0) THEN Erase(df);
  END;
  IF (NOT Error) AND s_del THEN Erase(sf);
  CopyFile:=Error;
END;

FUNCTION CompareDaffDateien(VAR d,s:TLDatei; Names:Boolean):Boolean;
VAR Error:Boolean;
    pss,psd:TLSignalPtr;
BEGIN
  Error:=FALSE;
  IF (d.blksizb<>s.blksizb) OR
     (d.blksize<>s.blksize) THEN
  BEGIN
    Writeln(^G'Dateien haben unterschiedliche Blockgrî·e');
    Error:=TRUE;
  END
  ELSE IF d.nsignale<>s.nsignale THEN
  BEGIN
    Writeln(^G'Dateien haben unterschiedlich viele Signale');
    Error:=TRUE;
  END
  ELSE BEGIN
    psd:=d.pSigs; pss:=s.pSigs;
    WHILE  (NOT Error) AND (psd<>NIL) DO
    BEGIN
      IF Names AND (psd^.name<>pss^.name) THEN
      BEGIN
        Writeln(^G'Dateien haben unterschiedliche SignalNamen: (',
                psd^.name,',',pss^.name,')');
        Error:=TRUE;
      END
      ELSE IF (psd^.dtyp <>pss^.dtyp) OR
              (psd^.wsize<>pss^.wsize) OR
              (psd^.size<>pss^.size) THEN
      BEGIN
        Writeln(^G'Signale haben unterschiedliche Datentype: (',
                psd^.name,',',pss^.name,')');
        Error:=TRUE;
      END
      ELSE IF (psd^.blkoffs<>pss^.blkoffs) OR
              (psd^.demux  <>pss^.demux) OR
              (psd^.wpblk  <>pss^.wpblk) THEN
      BEGIN
        Writeln(^G'Signale haben unterschiedliche Blockpositionen: (',
                psd^.name,',',pss^.name,')');
        Error:=TRUE;
      END
      ELSE IF Names THEN
      BEGIN
        IF psd^.x.fak<>pss^.x.fak THEN
        BEGIN
          Writeln(^G'Signale haben unterschiedliche Frequenz: (',
                  psd^.name,',',pss^.name,')');
          Error:=TRUE;
        END
        ELSE IF (psd^.y.fak<>pss^.y.fak) OR
                (psd^.y.off<>pss^.y.off) THEN
        BEGIN
          Writeln(^G'Signale haben unterschiedliche Einheiten: (',
                  psd^.name,',',pss^.name,')');
          Error:=TRUE;
        END
      END;
      psd:=psd^.pNext; pss:=pss^.pNext;
    END
  END;
  CompareDaffDateien:=Error
END;

VAR dst,src:TLDatei; e:DaffError;
    s_opt,n_opt,d_opt,v_opt,Flag:Boolean;
    DstName,SrcName,NewName,opt:PathStr;
    SrcOffset,DstOffset:LongInt; i:integer;
BEGIN

  IF (ParamCount<1) THEN usage;

  (* Voreingestellet Optionen *)
  s_opt:=FALSE; n_opt:=FALSE; d_opt:=FALSE;
  v_opt:=FALSE;
  DstName:=''; SrcName:=''; NewName:='';

  FOR i:=1 TO ParamCount DO
  BEGIN
    opt:=ParamStr(i);
    IF opt[1]='-' THEN
    BEGIN
     IF Length(opt)=2 THEN Flag:=TRUE
     ELSE IF Length(opt)=3 THEN BEGIN
      IF opt[3]='+' THEN Flag:=TRUE
      ELSE IF opt[3]='-' THEN Flag:=FALSE
      ELSE usage;
     END
     ELSE usage;
     CASE opt[2] OF
     'S','s' : s_opt:=Flag;
     'D','d' : d_opt:=Flag;
     'V','v' : v_opt:=Flag;
     ELSE usage;
     END
    END
    ELSE IF DstName='' THEN DstName:=opt
    ELSE IF SrcName='' THEN SrcName:=opt
    ELSE IF NewName='' THEN NewName:=opt
    ELSE usage
  END;

  n_opt:=NewName<>'';
  IF DstName='' THEN usage;
  IF SrcName='' THEN usage;
  IF NOT n_opt  THEN  NewName:=DstName;
  DefaultExtension(DstName,'.DAT');
  DefaultExtension(SrcName,'.DAT');
  DefaultExtension(NewName,'.DAT');

  IF v_opt THEN BEGIN
    writeln('AppenDAFF V1.0 (c) 1994 MDZ BÅhrer & Partner');
    writeln('--------------------------------------------');
    writeln(NewName,' = ',DstName,' + ',SrcName);
  END;

  IF v_opt THEN writeln('... Einlesen des Headers von ',DstName);
  e:=OpenDaffDatei(dst,DstName);
  IF e<>DAFF_NO_ERR THEN BEGIN
    Writeln(^G'DAFF-Fehler <',DstName,'> ',Ord(e),' = ',DaffErrorTexte[e]);
    HALT(1);
  END;
  IF v_opt THEN writeln('... Einlesen des Headers von ',SrcName);
  e:=OpenDaffDatei(src,SrcName);
  IF e<>DAFF_NO_ERR THEN BEGIN
    CloseDaffDatei(dst);
    Writeln(^G'DAFF-Fehler <',SrcName,'> ',Ord(e),' = ',DaffErrorTexte[e]);
    HALT(1);
  END;
  IF v_opt THEN writeln('... Vergleichen der beiden Header');
  IF CompareDaffDateien(dst,src,s_opt) THEN
  BEGIN
    CloseDaffDatei(dst);
    CloseDaffDatei(src);
    Halt(1);
  END;
  IF dst.xSet THEN
  BEGIN
    IF n_opt THEN BEGIN
      CloseDaffDatei(dst);
      CloseDaffDatei(src);
      Writeln(^G'Datei ',DstName,' hat interne LÑnge');
      HALT(1);
    END
    ELSE BEGIN
     IF v_opt THEN writeln('... RÅcksetzen der internen LÑnge');
     XResetDaffDatei(dst);
    END
  END;
  SrcOffset:=src.hdrsize;
  DstOffset:=DST_APPEND;
  CloseDaffDatei(dst);
  CloseDaffDatei(src);
  IF n_opt THEN
  BEGIN
    IF v_opt THEN writeln('... Kopieren von ',DstName,' nach ',NewName);
    IF CopyFile(DstName,0,NewName,0,d_opt) THEN HALT(1);
    IF v_opt AND d_opt THEN writeln('... Lîschen von ',DstName);
    DstName:=NewName;
  END;
  IF v_opt THEN writeln('... AnhÑngen von ',SrcName,' an ',DstName);
  IF CopyFile(SrcName,SrcOffset,DstName,DstOffset,d_opt) THEN HALT(1);
  IF v_opt AND d_opt THEN writeln('... Lîschen von ',SrcName);
  HALT(0);
END.


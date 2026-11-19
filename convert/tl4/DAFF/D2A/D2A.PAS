PROGRAM D2A; (* DAFF to ASCII Conversion *)
{$A+,B-,D+,E+,F+,G-,I-,L+,N+,O-,R-,S-,V+,X-}
{$M 4096,0,655360}

(*----------------------------------------------------------------+
 | Aufruf: D2A [-opts] daff[.dat] [asc[.asc] sigs[.lst]]          |
 +----------------------------------------------------------------+
 | Semantik:                                                      |
 |   liest die (binÑre) DAFF-Datei "daff" und gibt die Signale    |
 |   in lesbarer Form in die Datei "asc" aus. Gleichzeitig wird   |
 |   eine Datei "sigs" erstellt, in der die Namen und Einheiten   |
 |   der Signale aufgelistet sind;                                |
 |   Voreinstellungen:                                            |
 |     daff  - hat die Extension ".dat"                           |
 |     asc   - hat den selben Namen wie daff, extension ".ASC"    |
 |     sigs  - hat denselben Namen wie asc, extension ".LST"      |
 | Optionen:                                                      |
 |   -xn      gibt zusÑtzlich die X-Spalte aus                    |
 |            fÅr n=0 als Index (0..max), fÅr n=1 in [s],         |
 |            fÅr n=2 in [ms]                                     |
 |   -fn      Kontrolliert die Umrechnung (Voreinstellung: -f2)   |
 |            n=0 (in A/D-Einheiten), n=1 (in Volt), n=2 (in Usr) |
 |   -ln      Kontrolliert "sigs.lst" (Voreinstellung: -l2)       |
 |            n=0 (ohne) n=1 (auf CON:) n=2 (normal)              |
 |   -sn      Anzahl Stellen insgesamt (VE:15)                    |
 |   -dn      Anzahl Dezimalstellen (VE: E-Format)                |
 |   -tc      Trennzeichen (VE: Leerzeichen)                      |
 |   -t       Trennzeichen = Tab                                  |
 +----------------------------------------------------------------*)

USES Dos,TLData,TLDaff,TLDaten;

PROCEDURE usage;
BEGIN
 writeln('Aufruf: D2A [-opts] daff[.dat] [asc[.asc] sigs[.lst]]');
 writeln('    -xn      gibt zusÑtzlich die X-Spalte aus');
 writeln('             fÅr n=0 als Index (0..max), fÅr n=1 in [s],');
 writeln('             fÅr n=2 in [ms]');
 writeln('    -fn      Kontrolliert die Umrechnung (Voreinstellung: -f2)');
 writeln('             n=0 (in A/D-Einheiten), n=1 (in Volt), n=2 (in Usr)');
 writeln('    -ln      Kontrolliert "sigs.lst" (Voreinstellung: -l2)');
 writeln('             n=0 (ohne) n=1 (auf CON:) n=2 (normal)');
 writeln('    -sn      Anzahl Stellen insgesamt (VE:15)');
 writeln('    -dn      Anzahl Dezimalstellen (VE: E-Format)');
 writeln('    -tc      Trennzeichen (VE: Leerzeichen)');
 writeln('    -t       Trennzeichen = Tab');
 writeln;
 Halt(1);
END;

PROCEDURE DefaultExtension(VAR fn:PathStr; ext:ExtStr);
VAR
  d: DirStr;
  n: NameStr;
  e: ExtStr;
BEGIN
  FSplit(fn, d, n, e);
  IF E = '' THEN fn:=fn+ext;
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

PROCEDURE DoLstOpt(VAR fn:PathStr; VAR d:TLDatei; f:Integer);
VAR lstf:Text; s:TLSignalPtr; frqz:double; e:string;
BEGIN
   Assign(lstf,fn); Rewrite(lstf);
   writeln(lstf,'--- D2A ',d.path,' -----');
   writeln(lstf,'--- ',d.lable);
   writeln(lstf,d.nsignale,' Signale a ',d.nblocks,' Werten');
   s:=d.pSigs; frqz:=s^.x.fak;
   IF frqz<>0 THEN frqz:=1/frqz;
   writeln(lstf,frqz,' Samplingfrequenz');
   WHILE s<>NIL DO
   BEGIN
    CASE f OF
    0: e:='';
    1: e:=s^.ad.einh;
    2: e:=s^.y.einh;
    END;
    writeln(lstf,s^.name,' [',e,'] ',s^.lable);
    s:=s^.pNext;
   END;
   Close(lstf);
END;

PROCEDURE wrval(VAR f:Text;x:double;s,d:Integer; t:char; eoln:Boolean);
BEGIN
  IF d<0 THEN write(f,x:s)
  ELSE        write(f,x:s:d);
  IF eoln THEN writeln(f)
  ELSE         write(f,t);
END;

VAR d:TLDatei; e:DaffError;
    x_opt,f_opt,l_opt:Integer;
    s_opt,d_opt:Integer;
    t_opt:char;
    DaffName,AscName,LstName,opt:PathStr;
    x:double; i,n,err:integer;
    Lstf:Text; RecNo:LongInt;
BEGIN

  IF (ParamCount<1) THEN usage;

  x_opt:=-1; f_opt:=2; l_opt:=2;
  s_opt:=15; d_opt:=-1; t_opt:=' ';
  DaffName:=''; AscName:=''; LstNAme:='';

  FOR i:=1 TO ParamCount DO
  BEGIN
    opt:=ParamStr(i);
    IF opt[1]='-' THEN
    BEGIN
     IF opt[2] IN ['t','T'] THEN
     BEGIN
       IF Length(opt)=2 THEN t_opt:=chr(9)
       ELSE                  t_opt:=opt[3]
     END
     ELSE BEGIN
       Val(Copy(opt,3,10),n,err);
       IF (err<>0) OR (n<0) THEN usage;

       CASE opt[2] OF
       'X','x' : BEGIN
                  x_opt:=n;
                  if x_opt>2 THEN usage;
                 END;
       'F','f' : BEGIN
                  f_opt:=n;
                  if f_opt>2 THEN usage;
                 END;
       'L','l' : BEGIN
                  l_opt:=n;
                  if l_opt>2 THEN usage;
                 END;
       'S','s' : BEGIN
                  s_opt:=n;
                 END;
       'D','d' : BEGIN
                  d_opt:=n;
                 END;
       END
     END
    END
    ELSE IF DaffName='' THEN DaffName:=opt
    ELSE IF AscName=''  THEN AscName:=opt
    ELSE IF LstName=''  THEN LstName:=opt
    ELSE usage
  END;

  IF DaffName='' THEN usage;

  DefaultExtension(DaffName,'.DAT');
  IF AscName='' THEN
  BEGIN
    AscName:=NewExtension(DaffName,'.ASC');
    LstName:=NewExtension(DaffName,'.LST');
  END
  ELSE BEGIN
    DefaultExtension(AscName,'.ASC');
    IF LstName='' THEN LstName:=NewExtension(AscName,'.LST')
    ELSE DefaultExtension(LstName,'.LST');
  END;

  IF l_opt=1 THEN LstName:='';

  e:=OpenDaffDatei(d,DaffName);
  IF e=DAFF_NO_ERR THEN
  BEGIN
    IF l_opt>0 THEN DoLstOpt(LstName,d,f_opt);
    Assign(lstf,AscName); Rewrite(lstf);
    RecNo:=0;
    WHILE ReadNextRecord(d) DO
    BEGIN
      IF x_opt>=0 THEN
      BEGIN
       x:=QueryRecordTime(d,x_opt);
       wrval(lstf,x,s_opt,d_opt,t_opt,False);
      END;
      FOR i:=1 TO d.nsignale DO
      BEGIN
        x:=QuerySignalValue(d,i,f_opt);
        wrval(lstf,x,s_opt,d_opt,t_opt,i=d.nsignale);
      END;
      Inc(RecNo);
      IF (RecNo MOD 10)=0
      THEN write(RecNo,^M);
    END;
    Close(lstf);
    CloseDaffDatei(d);
    writeln;
  END
  ELSE BEGIN
    Writeln(^G'DAFF-Fehler ',Ord(e),' = ',DaffErrorTexte[e]);
    HALT(1);
  END
END.


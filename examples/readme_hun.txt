
egy program utasitasokbol all
minden utasitas vegere jon egy pontosvesszo
egy utasitas ugy nez ki hogy
	<muvelet> ;
	
a leg alapvetobb utasitas az egy ertek ataadas
az egyszeru egyenlo jel ertekatadast jelent
a bal oldalon egy valtozo kell legyen es a jobb oldalon  pedig barmi lehet
	
peldaul vegye fel x az y erteket
	x = y;
	
	
	valtozok
	
egy valtozo felvehet barmilyen erteket
vagy akar egy fuggvenyt is tartalmazhat
egy valtozo a kovetkezo karakterekbol allhat
	_abcdefg ... xyzABCDEFG ... XYZ01234 ... 9

de az elso karakter nem lehet szam

peldaul:
	alma = korte;
	i2 = x4 = y3 = z2;
	Szin = Fekete;
	

	tipusok
	
integer:
	egesz szamok
	pl. x = 13;
float:
	valos szamok
	pl. x = 3.49;
string:
	karakterlanc
	pl. x = "hello";
bit:
	egy binaris szam
	pl. x = bit(0);
	    x = bit(1);


	fuggvenyek meghivasa
	
egy utasitasba lehet egy fuggvenymeghivas
a fuggvenyek parametereket kapnak es altalaban visszateritenek egy erteket
	<fuggvenynev> ( <parameter>, <parameter>, ... )
	
a kovetkezo fuggvenyek leteznek, es ezen kivul letre lehet hozni ujjakat

view - kiiartas
	view(13+7); kiirtaja a 20-as szamot
	view(x,y,z); kiiratja a valtozok ertekeit
	
system - vegrehajt egy operacios rendszer utasitast
	system("cls"); kiuriti a kepernyot
	system("pause"); leszuneteli a programot
	system("dir"); kiiratja a fileokat a konyvtarban
	
exit - kilepes a programbol
	exit();
	exit(0); 
	exit(14); - visszaterit egy kilepesi kodot
	
type - visszateriti egy stringen az adott valtozo tipusat
	type(x); - visszateriti x tipusat
	view(type(x)); - kiiratja x tipusat
	
delete - kitorol egy valtozot
	delete("view"); - kirtorli a kiiratas fuggvenyt es tobbet nem lehet kiiratni

	
	muveletek
		
	tipus atalakitas

eggyik tipust at lehet alakitani egy masikra
pl. x = integer("16");
    x = string(16);
	x = float("16");
	x = bit(16); 
	
	
	aritmetika
	
ezek a muveletek ugyanazt a tipusu erteket hozzak letre

osszeadas
	x + y
	3 + 4
	0.3 + 0.5
	"szia " + "Tamas!"
	
kivonas
	x - y
	4 - 1
	1.0 - 0.5
	
szorzas
	x * y
	3 * 4
	0.5 * 2.0
	
osztas
	x / y
	19 / 10
	1.0 / 9.0
	
	
	osszehasonlitas
	
ezek a muveletek bit tipusu erteket hoznak letre 
ahol 1 - igazat jelent
	 0 - hamisat jelent
	 
egyenloseg
	1 == 1
	"a" == "a"
	
egyenlotlenseg
	1 != 2
	"a" != "b"
	
kissebb
	1 < 2
	"a" < "b"
	
nagyobb
	2 > 1
	"b" > "a"
	
kissebb vagy egyenlo
	2 <= 2
	1 <= 2
	"kutya" <= "kutya"

nagyobb vagy egyenlo
	2 >= 2
	2 >= 1
	"macska" >= "macska"

	
	felteteles vegrehajtas
	
az utasitas csak akkor van letrehajtva ha a feltetel igaz
a feltetel bit tipusu kell legyen (igaz vagy hamis)
	if ( <feltetel> ) <utasitas>

peldaul egy szam modulusza
	if (x<0) x = -x;
	
lehet rajni egy utasitast is ami hamis eseten hajtodik vegre
	if ( <feltete> ) utasitas; else utasitas;
	
peldaul ha egy valtozo 0 akkor 10-et rakunk bele, maskulomben csokkentsuk
	if (x==0) x=10; else x=x-1;


	ismetles
	
egy utasitast tobbszor hajt vegre
addig hajtodik vegre amig az utasitas hamis lesz
	while ( <feltetel> ) <utasitas>
	
peldaul elszamolni 10 ig:
	i=0; while(i<10) i=i+1;


	fuggvenyek
	
fuggvenyeket is letre lehet hozni
tobb utasitas be van csomagolva egy fuggvenybe
minden fuggveny visszateriti az utolso kiszamitott erteket
	function (<valtozo>, <valtozo>, ... ) <utasitas>
	
peldaul:
	dupla = function(x)x*2;;
	y = dupla(4);
	
	
	utasitas csoport

utasitasokat csoportositani lehet igy az felteteles vegrehajtas
ismetles vagy a fuggveny tobb mint egy utasitasbol allhat
	{ <utasitas> <utasitas> <utasitas> }
	
peldaul n faktorialis:
	s=1; n=9;
	while (n>0) { s=s*n; n=n-1; }

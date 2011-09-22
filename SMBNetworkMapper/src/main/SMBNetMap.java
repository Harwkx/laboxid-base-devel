package main;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

public class SMBNetMap {
	
	private BufferedReader br;
	private BufferedWriter bw;
	private String zeile = null;
	private String neueZeile = null;
	private String server = null;
	private String share = null;
	
	
	public static void main(String[] args)
	{
		//Header
		System.out.println(
				"####################################################\n"+
				"#                                                  #\n"+
				"#           SMBNetworkMapper alpha~0.0.1           #\n"+
				"#                  by 191cda1e                     #\n"+
				"#                                                  #\n"+
				"#           Releasedate: 05.09.2011                #\n"+
				"####################################################\n"
				);
		if(args.length < 1)
		{
			System.out.println("Usage: SMBNetMap.jar [--trimm | --fish] C:\\Pfad\\zu\\list.txt C:\\Pfad\\zu\\newlist.txt");
			System.exit(1);
		}
		File eingabeDatei = new File(args[1]);
		File ausgabeDatei = new File(args[2]);
		
		SMBNetMap lib = new SMBNetMap(eingabeDatei, ausgabeDatei);
		if(args[0].compareTo("--trimm") == 0)
		{
			lib.StartTrimming();
		}
		if(args[0].compareTo("--fish") == 0)
		{
			lib.StartFishing();
		}
		System.out.println("[+] Yo, alles fertig. Schließen...");
		System.exit(0);
	}
	
	
	public SMBNetMap()
	{
		this(null,null);
	}
	
	
	public SMBNetMap(File infile, File outfile)
	{
		try {
			this.br = new BufferedReader(new FileReader(infile));
		} catch (FileNotFoundException e) {
			System.out.println("[-] Datei konnte nicht gefunden werden.");
		}
		try {
			this.bw = new BufferedWriter(new FileWriter(outfile));
		} catch (IOException e) {
			System.out.println("[-] Output Datei konnte nicht initialisiert werden.");
		}
	}
	
	
	public void StartFishing()
	{
		try
		{
			zeile = br.readLine();
			do
			{
				int start = zeile.indexOf("\\\\");
				if(start != -1)
				{
					int end = zeile.length();
					server = zeile.substring(start, end);
					server = server.trim();
					zeile = br.readLine();
					if(zeile.contains("Freigegebene"))
					{
						for(int i=0;i<6;i++)
						{
							br.readLine();
						}
						zeile = br.readLine();
						while(!(zeile.contains("Der Befehl wurde erfolgreich")))
						{
							if(zeile.contains("Platte"))
							{
								start = 0;
								end = zeile.indexOf("Platte");
								share = zeile.substring(start, end);
								share = share.trim();
								neueZeile = server.concat("\\").concat(share);
								bw.write(neueZeile);
								bw.newLine();
							}
							zeile = br.readLine();
						}
					}
					
				}
				zeile = br.readLine();
			}while(zeile != null);
			bw.close();
			br.close();
		}
		catch (IOException e)
		{
				System.out.println("[-] Fehler beim schreiben. Du hast es kaputt gemacht!");
		}
		System.out.println("[+] Filtern und schreiben erfolgreich abgeschlossen.");
	}
	
	
	public void StartTrimming()
	{
		try
		{
			String zeile = null;
			String neueZeile = null;
			zeile = br.readLine();
			do
			{
				int start = zeile.indexOf("\\\\");
				int end = zeile.indexOf(" ");
				if(end == -1)
				{
					end = 999;
				}
				int end2 = zeile.indexOf("\t");
				if(end2 == -1)
				{
					end2 = 999;
				}
				if(end2 == end)
				{
					end = zeile.length();
				}
				if(end2 < end)
				{
					end = end2;
				}
				neueZeile = zeile.substring(start, end);
				bw.write(neueZeile);
				bw.newLine();
				zeile = br.readLine();
			}while(zeile != null);
			bw.close();
			br.close();
		}
		catch (IOException e)
		{
				System.out.println("[-] Fehler beim schreiben. Du hast es kaputt gemacht!");
		}
		System.out.println("[+] Alles wurde sauber getrennt. Wenn nicht, bist du selbst schuld.");
	}
}
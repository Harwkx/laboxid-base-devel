#include <stdio.h>
#include <string.h>
#include <malloc.h>

struct dbObject
{
    unsigned char value;
    unsigned int entropy;
    float percentage;
};

int main()
{
    struct dbObject db[256];
    struct dbObject tmp;
    FILE* fp;
    long size=0;
    unsigned char highest = 0;
    unsigned char lowest = 0;
    unsigned char c = 0;
    char* reportFile=0;
    //unsigned char janitor = 0;
    char* file = (char*) malloc((sizeof(char)*MAX_PATH));
    memset(file,0,MAX_PATH);

    //Datenbank initialisieren
    for(int i = 0;i<256;i++)
    {
        db[i].value=i;
        db[i].entropy=0;
        db[i].percentage=0.0f;
    }

    printf("\nZu untersuchende Datei bitte eingeben: ");
    gets(file);

    reportFile = (char*)malloc(sizeof(char)*strlen(file)+strlen("-file entropy report.txt"));
    strcpy(reportFile,file);
    strcat(reportFile,"-file entropy report.txt");

    fp = fopen(file,"rb");

    //Dateigröße in Bytes ermitteln
    fseek(fp,0L,SEEK_END);
    size=ftell(fp);
    rewind(fp);

    //Entropie-Daten einlesen
    while(!feof(fp))
    {
         c = fgetc(fp);
         db[c].entropy++;
    }
    fclose(fp);

    //Datenbank sortieren: Insertionsort
    for(int i=1;i<256;i++)
    {
        tmp.entropy=db[i].entropy;
        tmp.value=db[i].value;
        int j = i;
        while(j>0 && db[j-1].entropy < tmp.entropy)
        {
            db[j] = db[j-1];
            j=j-1;
        }
        db[j].entropy = tmp.entropy;
        db[j].value = tmp.value;
    }

    //höchste Entropie
    highest=db[255].value;

    //geringste Entropie
    lowest=db[0].value;

    //Prozente ausrechnen
    for(int i=0;i<256;i++)
    {
        db[i].percentage=(float)db[i].entropy/size*100.0f;
    }

    fp = fopen(reportFile,"w");
    fprintf(fp,"File Entropy Report\n==================================\n\n");
    fprintf(fp,"File: <%s>\n\n",file);
    fprintf(fp,"Character\t|| Entropy\t|| Percentage\n");
    fprintf(fp,"=================================================\n");

    for(int i=0;i<256;i++)
    {
        fprintf(fp,"%i\t\t|| %u\t\t|| %.2f%%\n",db[i].value,db[i].entropy,db[i].percentage);
    }


    fprintf(fp,"Highest Entropy: %i\n",highest);
    fprintf(fp,"Lowest Entropy : %i\n",lowest);

    fclose(fp);


    free(file);
    return 0;
}

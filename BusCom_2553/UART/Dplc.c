#include <msp430.h>
#define VAL_MAX_RC 500


int vitesse1, vitesse2, FLG1 = 0 , FLG2 = 0, FG=0, FG1=0, tour_bis =0, tour_bis1 =0;

void Set_vitesse (int vitss1,int vitss2)//COntrôle la vitesse des 2 moteurs
{
    if(vitss1<=VAL_MAX_RC && vitss2<=VAL_MAX_RC)
    {
        TA1CCR1=vitss1-vitss1*0.06;//Moteur Gauche
        TA1CCR2=vitss2;//Moteur Droit
    }

}

void Set_vitesse_Mot_1 (int vitss)//contrôle la vitesse du moteur 1
{
    if(vitss<=VAL_MAX_RC )
    {
        TA1CCR1=vitss;//Moteur Gauche
    }

}

int Get_vitesse_Mot_1()
{
    return TA1CCR1;
}


void Set_vitesse_Mot_2 (int vitss)//contrôle la vitesse du moteur 2
{
    if(vitss<=VAL_MAX_RC)
    {
        TA1CCR2=vitss;//Moteur Droit
    }
}

int Get_vitesse_Mot_2()
{
    return TA1CCR2;
}


void Set_Direction (char dir)//Permet de choisir la direction
{
    int i;

    Set_vitesse(0,0);

    if(dir==1)//AVANCER
    {
        P2OUT |= BIT5;
        P2OUT &= ~BIT1;
    }
    else if (dir==2)//RECULER
    {
        P2OUT |= BIT1;
        P2OUT &= ~BIT5;
    }
    else if (dir==3)//DROITE
        {
        P2OUT |= BIT5;
        P2OUT |= BIT1;
        }
    else if (dir==4)//GAUCHE
        {
        P2OUT &= ~BIT5;
        P2OUT &= ~BIT1;
        }

    Set_vitesse(100,100);

}


void Init_robot()//fonction d'initialisation du robot
{
        WDTCTL = WDTPW + WDTHOLD;
        BCSCTL1= CALBC1_1MHZ;      //frequence d’horloge 1MHz
        DCOCTL= CALDCO_1MHZ;        //  "

        //1er roue A
        //vitesse
        P2DIR |= BIT2;             // P2.2 en sortie
        P2SEL |= BIT2;             // selection fonction TA1.1
        P2SEL2 &= ~BIT2;          // selection fonction TA1.1 (suite)

        //sens roues
        P2DIR |= BIT1;             //P2.1 en sortie pour le sens de variation de la roue
        P2SEL &= ~BIT1;             // en mode O/I
        P2SEL2 &= ~BIT1;             //sortie
        P2REN |=BIT1;           //activation de la resistance de tirage
        P2OUT |=BIT1;

        //opto
        P2DIR &= ~BIT0;             //P2.0 en entrée pour l'opto
        P2SEL &= ~BIT0;             // en mode O/I
        P2SEL2 &= ~BIT0;            //entrée



        //2éme roue B
        //vitesse
        P2DIR |= BIT4;             // P2.4 en sortie
        P2SEL |= BIT4;             // selection fonction TA1.1
        P2SEL2 &= ~BIT4;          // selection fonction TA1.1 (suite)

        //sens roues
        P2DIR |= BIT5;             //P2.5 en sortie pour le sens de variation de la roue
        P2SEL &= ~BIT5;             // en mode O/I
        P2SEL2 &= ~BIT5;
        P2REN |=BIT5;           //activation de la resistance de tirage
        P2OUT |=BIT5;

        //opto
        P2DIR &= ~BIT3;             //P2.3 en entrée pour l'opto
        P2SEL &= ~BIT3;             // en mode O/I
        P2SEL2 &= ~BIT3;            //entrée



        TA1CTL = TASSEL_2 | MC_1 | ID_0;  // source SMCLK pour TimerA (no 1), mode comptage Up
        //TA1CTL|=TAIE;//autorisationinterruptionTAIE
        TA1CCTL1 |= OUTMOD_7; // activation mode de sortie n°7
        TA1CCTL2 |= OUTMOD_7;

        TA1CCR0 = VAL_MAX_RC;            // determine la periode du signal
}



/*void regulation(int cpt1, int copt2)
{
    int FG1=0;

            //1me roues
        if((P2IN & BIT0) == BIT0 && FG==1)
           FG1=1;

        else if((P2IN & BIT0) != BIT0 && FG==0)
           FG1=2;



            //2ME ROUES
        else if((P2IN & BIT3) == BIT3 && FG1==0)
           FG1=1;

        else if((P2IN & BIT3) != BIT3 && FG1==1)
            FG1=0;

}*/

int val_optocoupleur_Mot_1()
{

    //1me roues
    if((P2IN & BIT0) == BIT0 && FLG1==0)
      {
       FLG1=1;
       return 1;
      }
    else if((P2IN & BIT0) != BIT0 && FLG1==1)
      {
        FLG1=0;
        return 0;
      }

    return 3;
}

int val_optocoupleur_Mot_2()
{

        //2ME ROUES
    if((P2IN & BIT3) == BIT3 && FLG2==0)
      {
       FLG2=1;
       return 1;
      }
    else if((P2IN & BIT3) != BIT3 && FLG2==1)
      {
        FLG2=0;
        return 0;
      }
    return 3;
}

void Equilibre()// permet de mettre les 2 roue dans la même position (dans un "trou") avec les optos
{
    if(val_optocoupleur_Mot_1() != val_optocoupleur_Mot_2())
    {
        Set_vitesse (0,0);

        while((P2IN & BIT0) == BIT0)//Roue gauche
        {

            Set_vitesse (VAL_MAX_RC*0.1,0);

        }
        while((P2IN & BIT3) == BIT3)//Roue gauche
        {

            Set_vitesse (0,VAL_MAX_RC*0.1);

        }
    }

}

//fonction permettant de compter un nombre de pas d'opto
int Lecture_nbr_tour(int nbr)
{
    tour_bis = 0;
    tour_bis1 = 0;

    while(tour_bis<nbr);

    if(tour_bis==nbr)
    {
        tour_bis=0;
        tour_bis1 = 0;
        FG = 0;
        FG1 = 0;
        //P2IE &= ~(BIT0|BIT3);
        return 1;
    }
    else
        return 0;
}

#pragma vector = PORT2_VECTOR
__interrupt void coupleA_compte(){

    if((P2IFG & BIT0)==BIT0){
        tour_bis++;
        P2IFG &=~BIT0;
    }
    if((P2IFG & BIT3)==BIT3){
           tour_bis1++;
           P2IFG &=~BIT3;
       }
}

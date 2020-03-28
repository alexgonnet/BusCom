#include <msp430.h>
#include "adc.h"
#include "Dplc.h"
#include "capteurs.h"
#define RECULER 1
#define AVANCER 2
#define GAUCHE 3
#define DROITE 4

#define TIME 30

//Moteur GAUCHE est le MOTEUR_1
//Moteur DROITE est le MOTEUR_2



unsigned int compteur=0, tolerance = 0, move = 0, trou = 0, obstacle = 0;
int  infra_centre= 0, infra_droite = 0;
int capt= 0, init = 0;
//Fonction permettant de compter un certain nombre de tour


int main(void)

{

    int i;

    Init_robot();
    ADC_init();

    //interrupteur pour interruption d'arrêt
    P1DIR &= ~(BIT0|BIT6|BIT3);             //P1.3 en entrée pour l'interrupteur
    P1SEL &= ~BIT3;             // en mode O/I
    P1SEL2 &= ~BIT3;            //entrée
    P1REN |=BIT3;           //activation de la resistance de tirage
    P1OUT |=BIT3;           //PULL-UP

    P1DIR |= BIT2 | BIT7;             //P1.2 en sorti led
    P1SEL &= ~BIT2;             // en mode O/I
    P1SEL2 &= BIT2;            //entrée

    //INTERRUPTION
    //interrupteur
    P1IE |= BIT3;//selectionne le port de l'interruption
    P1IES |= BIT3;//selectionne le front
    P1IFG &= ~(BIT3);//descendre le flag

    P2IE |= BIT0|BIT3;
    P2IES |= BIT0|BIT3;


    P2DIR &= ~BIT0;             //P1.3 en entrée pour l'interrupteur
    P2SEL &= ~BIT0;             // en mode O/I
    P2SEL2 &= ~BIT0;            //entrée
    //timer pour asservissement
    TA0CTL=0|(TASSEL_2|ID_3);//sourceSMCLK,pasdepredivisionID_0
    TA0CTL|=MC_3;//comptageenmodeupdown
    TA0CTL|=TAIE;//autorisationinterruptionTAIE
    TA0CCR0=62500;//periode timer pour 1s


    __enable_interrupt();//Activation des interruptions

while(1)//boucle sans fin
{
    while(move != 0)// boucle qui permet d'attendre l'appuie sur S2 pour démarrer et d'arrêter le robot sous certaines conditions
    {
        if(init == 0) //conditions exécutées seulement au démarrage
        {
            Set_Direction(AVANCER); // définit la direction devant
            if(infra_centre <= -15 && infra_droite <= -15) //si on ne détecte aucun obstacle
            {
                tolerance = 1; // on autorise le robot à rouler droit jusqu'à ce qu'il trouve un obstacle
               /* while(infra_centre == 0 && infra_droite == 0);
                {
                    ADC_Demarrer_conversion(0x00);
                        if(ADC_Lire_resultat() > 600)
                            infra_centre = 1;
                        else
                            infra_centre = 0;

                        ADC_Demarrer_conversion(0x06);
                        if(ADC_Lire_resultat() > 500)
                            infra_droite = 1;
                        else
                            infra_droite = 0;
                }
                Set_Direction(GAUCHE);
                while(!Lecture_nbr_tour(6));
                Set_Direction(AVANCER);*/
            }
            init = 1; // indique que l'initialisation a été lancée
        }
//HOMOLOGUATION
        Set_Direction(AVANCER);

        while(!Lecture_nbr_tour(50));
        for(i=0;i<10;i++)
            Equilibre();

        Set_Direction(DROITE);
        while(!Lecture_nbr_tour(6));
        for(i=0;i<10;i++)
            Equilibre();

        Set_Direction(AVANCER);
        while(!Lecture_nbr_tour(50));
        for(i=0;i<10;i++)
            Equilibre();


        Set_Direction(DROITE);
        while(!Lecture_nbr_tour(6));
        for(i=0;i<10;i++)
            Equilibre();

        Set_Direction(AVANCER);
        while(!Lecture_nbr_tour(50));
        for(i=0;i<10;i++)
            Equilibre();

        Set_Direction(DROITE);
        while(!Lecture_nbr_tour(6));
        for(i=0;i<10;i++)
            Equilibre();

        Set_Direction(AVANCER);
        while(!Lecture_nbr_tour(50));
        for(i=0;i<10;i++)
            Equilibre();

        Set_Direction(DROITE);
        while(!Lecture_nbr_tour(6));
        for(i=0;i<10;i++)
            Equilibre();

      /*  if(trou == 1 || obstacle == 1)
        {
            Set_Direction(RECULER);
            //Equilibre();
            while(!Lecture_nbr_tour(2));
            Set_Direction(GAUCHE);
           // Equilibre();
            while(!Lecture_nbr_tour(6));
            Set_Direction(AVANCER);
            while(infra_centre == 0);
            Set_Direction(GAUCHE);
            //Equilibre();
            while(!Lecture_nbr_tour(6));

        }
        else*/
        if(tolerance != 0 && infra_droite >= 50) // si le robot est autorisé à aller droit et qu'on a un obstacle a droite
        {
                tolerance = 0; // on enlève l'autorisation
        }
        else if(infra_centre >= 50)// si on a un obstacle devant
        {
            Set_Direction(GAUCHE); // on tourne a gauche
           // Equilibre();
            while(!Lecture_nbr_tour(6));
               // Equilibre();
            Set_Direction(AVANCER);
            infra_centre = 0; // on remet le compteur de valeurs capteur à 0

        }
        else if(tolerance == 0 && infra_droite <=-50) // si le robot n'est pas autorisé à aller droit et qu'on a pas d'obstacle a droite
        {
            while(!Lecture_nbr_tour(2)); // avance légèrement pour être à hauteur de l'ouverture à droite
            Set_Direction(DROITE); //tourne a droite
            while(!Lecture_nbr_tour(6));
            //for(i=0;i<10;i++)
              // Equilibre();
            Set_Direction(AVANCER); //on avance
            tolerance = 1;// on autorise le robot a avancer sans obstacle a droite
            infra_droite = 0; // on remet le compteur de valeurs capteur à 0
           /* while(infra_centre == 0 && infra_droite == 0);
            {
                ADC_Demarrer_conversion(0x00);
                if(ADC_Lire_resultat() > 600)
                    infra_centre = 1;
                else
                    infra_centre = 0;

                ADC_Demarrer_conversion(0x06);
                if(ADC_Lire_resultat() > 500)
                    infra_droite = 1;
                else
                    infra_droite = 0;
            }*/
        }


        }

}
}


#pragma vector=PORT1_VECTOR//interrupteur d'arrêt d'urgence
__interrupt void Interrupteur(void)
{
    if(move == 0) // interrupteur a off
    {
        move = 1; // on démarre le fonctionnement
    }
    else if( init != 0)  // interrupteur a on
    {
        Set_vitesse(0,0); // on arrete le robot
    }

    P1IFG &= ~(BIT3);
}

#pragma vector=TIMER1_A1_VECTOR //timer utilisé pour lire les données des capteurs
__interrupt void detect(void)
{
    if(move == 1) // si le robot est en fonctionnement
    {
        P1OUT |= BIT7;
        __delay_cycles(10);
        P1OUT &= ~BIT7;
        P1IE |= BIT6;
        P1IES &= ~BIT6;
        __delay_cycles(30000)
        //Equilibre();
        ADC_Demarrer_conversion(0x00); // on lit l'infrarouge du centre
        if(ADC_Lire_resultat() > 500) // si la valeur est dessus du seuil, on a un obstacle
        {
            if(infra_centre < 51) // on vérifie plusieurs fois la valeur pour éviter les erreurs
                infra_centre++;

        }
        else
        {
            if(infra_centre > -51)
                infra_centre--;

        }


        ADC_Demarrer_conversion(0x06);// on lit l'infrarouge de droite
        capt =ADC_Lire_resultat();
        if(capt > 450)
        {
            if(infra_droite < 51)
                infra_droite++;
          /*  if( init = 1 && tolerance == 0 && capt > 550)
                Set_vitesse(150, 100);
            Set_vitesse(100, 150);
            Set_vitesse(100, 100);*/


        }
        else
        {
            if(infra_droite > -51)
                infra_droite--;
           // if( tolerance == 0 && capt < 350)
           //     Set_vitesse(100, 150);

        }



        /*ADC_Demarrer_conversion(0x07);
        if(ADC_Lire_resultat() > 800)
            light = 1;
        else
            light = 0;*/
        }


    TA1CTL &= ~TAIFG; //RAZ TAIFG
}
#pragma vector=TIMER0_A1_VECTOR //timer utilisé pour compter les sec
__interrupt void timer_test (void)
{
    if(move == 1 ) // si le robot est en fonctionnement, on compte les secondes
        compteur++;
    if(compteur > TIME) // si le temps est supérieur a la durée max
    {
        move = 0;// on arrête le robot
        __disable_interrupt(); // on désactive les interruptions pour éviter qu'il reparte
        Set_vitesse(0,0);
    }
    TA0CTL &= ~TAIFG; //RAZ TAIFG
}

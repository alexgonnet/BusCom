/*
 * capteurs.c
 *
 *  Created on: 23 nov. 2019
 *      Author: GONNET
 */

int isMurDevant(void)
{
    ADC_Demarrer_conversion(0x00);
    int infra_centr = ADC_Lire_resultat();
    if(infra_centr > 600)
        return 1;
    else
        return 0;
}

int isMurDroite(void)
{
    ADC_Demarrer_conversion(0x06);
    int infra_droite = ADC_Lire_resultat();
    if(infra_droite > 500)
        return 1;
    else
        return 0;
}

int isObstacleDevant(void)
{

}

int isTrou(void)
{

}

int isArrive(void)
{

}


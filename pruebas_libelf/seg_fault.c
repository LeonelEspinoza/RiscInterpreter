/*
seg_fault.c

Crea un arreglo e intenta acceder a un espacio fuera del mismo.
Busca generar un error de Segmentation Fault
*/
int main(){
    int ar[5];
    int i=0;
    
    while(i!=5){
        ar[i]=0;
        i+=1000;
    }
    
    return ar[0];
}
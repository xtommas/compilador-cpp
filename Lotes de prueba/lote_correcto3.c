void main()
{
    int n; 
    int t1 = 0;
    int t2 = 1; 
    int nextTerm = 0;
    int i= 1;

    cout << "Ingrese el numero de terminos: ";
    cin >> n;

    cout << "Sucesion de Fibonacci: ";

    while (i<=n){
        if(i == 1)
        {
            cout << " " << t1 << " ";
        }
        else{
            if(i == 2)
            {
                cout << t2 << " ";
            }
            else{
                nextTerm = t1 + t2;
                t1 = t2;
                t2 = nextTerm;
        
                cout << nextTerm << " ";
            }
        }
        
        i = i + 1;
    }
    
}

void main() 
{
    int exponent;
    float base;
    float result = 1.0;

    cout << "Ingrese la base y el exponente respectivamente: ";
    cin >> base >> exponent;

    cout << base << "^" << exponent << " = ";

    while (exponent != 0) {
        result = result * base;
        exponent = exponent - 1;
    }

    cout << result;
    
}

#ifndef EXTPROGRAM_H
#define EXTPROGRAM_H


class ExtProgram
{
public:
    enum Id {
        sox = 1,
    };
    Q_ENUM(Id);


    ExtProgram();
};

#endif // EXTPROGRAM_H

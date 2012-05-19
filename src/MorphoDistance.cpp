#include "MorphoDistance.h"

#include "BinaryMorphology.h"

#include <QDebug>
const int MorphoDistance::SE_SQUARE_ARR[] = {1, 1, 1,
                                             1, 1, 1,
                                             1, 1, 1};

const int MorphoDistance::SE_CROSS_ARR[] = {0, 1, 0,
                                            1, 1, 1,
                                            0, 1, 0};

MorphoDistance::MorphoDistance(ImageProcessor* imageProcessor) :
    imageProcessor(imageProcessor),
    distanceImage(0),
    skeletonImage(0),
    useSquareSe(true)
{
}

MorphoDistance::~MorphoDistance()
{
    if (distanceImage) {
        delete distanceImage;
    }
    if (skeletonImage) {
        delete skeletonImage;
    }
}

bool MorphoDistance::isUseSquareSe() const
{
    return useSquareSe;
}

QImage* MorphoDistance::getDistanceImage(const bool useSquareSe)
{
    this->useSquareSe = useSquareSe;
    StructElement* se;
    if (useSquareSe) {
        se = new StructElement(SE_RATIO, SE_RATIO, 1, 1, SE_SQUARE_ARR);
    } else {
        se = new StructElement(SE_RATIO, SE_RATIO, 1, 1, SE_CROSS_ARR);
    }

    BinaryMorphology morpho(imageProcessor);
    QImage* origin = morpho.getOperatedImage();
    int size = origin->width() * origin->height();

    int* stepArr = new int[size];
    for (int i = 0; i < size; ++i) {
        stepArr[i] = 0;
    }

    // set each border to be current step
    int step = 0;
    QImage* lastImage = origin;
    QImage* curImage;
    uchar fore = morpho.getForeground();
    uchar back = morpho.getBackground();
    while (true) {
        ++step;
        curImage = morpho.doErosion(*se);

        // mark border to be step
        const uchar* cBits = curImage->constBits();
        const uchar* lBits = lastImage->constBits();
        for (int i = 0; i < size; ++i) {
            if (*cBits == back && *lBits == fore) {
                // find border, mark with step
                stepArr[i] = step;
            }
            // to next pixel
            cBits += 4;
            lBits += 4;
        }

        if (morpho.isAllBack()) {
            break;
        }
        lastImage = curImage;
    }

    // new distance image according to stepArr
    double ratio;
    if (step == 0) {
        ratio = 1.0;
    } else {
        ratio = 255.0 / step;
    }
    if (distanceImage) {
        delete distanceImage;
    }
    distanceImage = new QImage(origin->size(), origin->format());
    uchar* dBits = distanceImage->bits();
    for (int i = 0; i < size; ++i) {
        for (int rgb = 0; rgb < 3; ++rgb) {
            *(dBits + rgb) = stepArr[i] * ratio;
        }
        dBits += 4;
    }

    delete se;
    delete []stepArr;
    return distanceImage;
}

QImage* MorphoDistance::getSkeletonImage()
{
    StructElement se(SE_RATIO, SE_RATIO, SE_RATIO / 2, SE_RATIO / 2,
                     SE_CROSS_ARR);

    QImage* origin = imageProcessor->getBinaryImage();
    int size = origin->width() * origin->height();
    uchar back = BinaryMorphology::DEFAULT_BACK_COLOR;
    uchar fore = BinaryMorphology::DEFAULT_FORE_COLOR;


    // A ero kB
    QImage* ero = origin;
    BinaryMorphology morpho(ero);
    // sigma(Sk(A))
    uchar* sigmaSk = new uchar[size];
    for (int i = 0; i < size; ++i) {
        sigmaSk[i] = back;
    }

    // do while (A ero kB) is not empty
    while (true) {
        // (A ero kB) open B
        QImage* open = morpho.doOpening(se);
        morpho.undo();

        // Sk(A) = ero - open
        // sigma(Sk(A)) = Sk(A) + sigma(S(k-1)(A))
        const uchar* eBits = ero->bits();
        const uchar* oBits = open->bits();
        for (int i = 0; i < size; ++i) {
            // union current foreground
            if (*eBits == fore && *oBits == back) {
                sigmaSk[i] = fore;
            }
            eBits += 4;
            oBits += 4;
        }

        QImage tmp(origin->size(), origin->format());
        uchar* sBits = tmp.bits();
        const uchar* Bits = origin->constBits();
        for (int i = 0; i < size; ++i) {
            uchar value = back;
            if (*Bits && !sigmaSk[i]) {
                value = fore;
            }
            for (int rgb = 0; rgb < 3; ++ rgb) {
                *(sBits + rgb) = value;
            }
            sBits += 4;
            Bits += 4;
        }

        // to next k
        ero = morpho.doErosion(se);
        if (morpho.isAllBack()) {
            break;
        }

    }

    if (skeletonImage) {
        delete skeletonImage;
    }
    skeletonImage = new QImage(origin->size(), origin->format());
    uchar* sBits = skeletonImage->bits();
    const uchar* oBits = origin->constBits();
    // skeleton is sigmaSk
    for (int i = 0; i < size; ++i) {
        for (int rgb = 0; rgb < 3; ++ rgb) {
            *(sBits + rgb) = sigmaSk[i];
        }
        sBits += 4;
        oBits += 4;
    }

    delete []sigmaSk;
    return skeletonImage;
}

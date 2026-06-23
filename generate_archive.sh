PROJECT="jeffmanzione_language_tools"
TAG="v1.0.x"
VERSION="1.0.2"

git archive \
    --format=zip \
    --prefix "$PROJECT-$VERSION/" \
    --output "./$PROJECT-$VERSION.zip" \
    "$TAG"

git archive \
    --format="tar.gz" \
    --prefix "$PROJECT-$VERSION/" \
    --output "./$PROJECT-$VERSION.tar.gz" \
    "$TAG"
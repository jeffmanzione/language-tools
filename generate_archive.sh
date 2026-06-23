PROJECT="jeffmanzione_language_tools"
TAG="v0.0.x"
VERSION="0.0.0"

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
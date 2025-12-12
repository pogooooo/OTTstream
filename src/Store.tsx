import { configureStore, combineReducers } from '@reduxjs/toolkit';
import { persistReducer, persistStore } from 'redux-persist';
import storage from 'redux-persist/lib/storage'; // 로컬 스토리지 사용 (기본)
// import storageSession from 'redux-persist/lib/storage/session'; // 세션 스토리지 사용 원할 시

import userReducer from "./slices/UserSlice.tsx";

const rootReducer = combineReducers({
    user: userReducer,
});

const persistConfig = {
    key: 'root',
    storage,
    whitelist: ['user'],
    // blacklist: ['loading'], // 반대로 저장하지 않을 것들을 적어도 됨
};

const persistedReducer = persistReducer(persistConfig, rootReducer);

const store = configureStore({
    reducer: persistedReducer,
    middleware: (getDefaultMiddleware) =>
        getDefaultMiddleware({
            serializableCheck: {
                ignoredActions: ['persist/PERSIST', 'persist/REHYDRATE'],
            },
        }),
});

export const persistor = persistStore(store);
export default store;
export type RootState = ReturnType<typeof store.getState>;
